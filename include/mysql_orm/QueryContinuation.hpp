#ifndef MYSQL_ORM_QUERYCONTINUATION_HPP_
#define MYSQL_ORM_QUERYCONTINUATION_HPP_

#include <functional>

#include <mysql/mysql.h>

#include <mysql_orm/Statement.hpp>

namespace mysql_orm
{
/** Defines common methods for continuation queries.
 *
 * We call "continuation queries" classes that add to another query class (such
 * as `Set` to `Update`, or `Limit` to `Where`). `Select` and `Update` are not
 * continuation queries.
 *
  * The class inherits the Continuation given in template argument. For
  * instance, when implementing Where, an implementation class `WhereImpl` (for
  * instance templated over a `Select` instanciation, which we call `Query`)
  * should contain methods specific to the Where clause, and a `WhereQuery`
  * alias may be created on `QueryContinuation<Query, Table, WhereImpl<...>>`.
 *
 * This class defines:
 *   - `buildquery`: Returns the SQL query as a std::string.
 *   - `getNbInputSlots`: Returns the number of input slots the class (and
 *     parents) needs.
 *   - `getNbOutputSlots`: Returns the number of output slots the class (and
 *     parents) needs.
 *   - `bindInTo`: Binds input slots of the class (and parents).
 *   - `bindOutTo`: Binds output slots of the class (and parents).
 *   - `finalizeBindings`: Performs last-minute operations on fields before
 *     copying.
 *
 * The methods `getNbInputSlots` and `bindInTo` are handled particularly.
 * If one exists in `Continuation`, `QueryContinuation` will use this one. It
 * will otherwise directly forward the call to `Query`.
 */
template <typename Query, typename Table, typename Continuation>
class QueryContinuation : public Continuation
{
public:
  using model_type = typename Query::model_type;
  static inline constexpr auto is_select_query{Query::is_select_query};

  template <typename... ContinuationArgs>
  QueryContinuation(MYSQL& mysql,
                    Query q,
                    Table const& t,
                    ContinuationArgs... args) noexcept
    : Continuation{
          mysql, std::move(q), t, std::forward<ContinuationArgs>(args)...}
  {
  }
  QueryContinuation(QueryContinuation const& b) noexcept = default;
  QueryContinuation(QueryContinuation&& b) noexcept = default;
  ~QueryContinuation() noexcept = default;

  QueryContinuation& operator=(QueryContinuation const& rhs) noexcept = default;
  QueryContinuation& operator=(QueryContinuation&& rhs) noexcept = default;

  std::string buildquery() const
  {
    return this->buildqueryss().str();
  }

  size_t getNbInputSlots() const noexcept
  {
    return this->getNbInputSlotsImpl(*this, 0);
  }

  size_t getNbOutputSlots() const noexcept
  {
    return this->query.getNbOutputSlots();
  }

  void bindInTo(MYSQL_BIND* bindarray) const noexcept
  {
    this->bindInToImpl(*this, bindarray, 0);
  }

  void bindOutTo(model_type& model, std::vector<MYSQL_BIND>& bindarray) const
  {
    this->query.bindOutTo(model, bindarray);
  }

  void finalizeBindings(model_type& model,
                        std::vector<MYSQL_BIND>& binds,
                        std::vector<unsigned long>& lengths,
                        std::vector<my_bool>& are_null,
                        std::vector<my_bool>& errors)
  {
    this->query.finalizeBindings(model, binds, lengths, are_null, errors);
  }

  Statement<QueryContinuation, model_type> build() const
  {
    return Statement<QueryContinuation, model_type>{*this->mysql_handle, *this};
  }

private:
  template <typename Q>
  static auto getNbInputSlotsImpl(Q const& q, int) noexcept
      -> decltype(q.Continuation::getNbInputSlots(), size_t())
  {
    return q.Continuation::getNbInputSlots();
  }
  template <typename Q>
  static size_t getNbInputSlotsImpl(Q const& q, long) noexcept
  {
    return q.query.getNbInputSlots();
  }

  template <typename Q>
  static auto bindInToImpl(Q const& q,
                           MYSQL_BIND* bindarray,
                           int) noexcept
      -> decltype(q.Continuation::bindInTo(bindarray), void())
  {
    q.Continuation::bindInTo(bindarray);
  }
  template <typename Q>
  static void bindInToImpl(Q const& q,
                           MYSQL_BIND* bindarray,
                           long) noexcept
  {
    q.query.bindInTo(bindarray);
  }
};

template <typename Query, typename Table, typename Continuation>
QueryContinuation(Query, Table, Continuation)
    ->QueryContinuation<Query, Table, Continuation>;
}

#endif /* !MYSQL_ORM_QUERYCONTINUATION_HPP_ */
