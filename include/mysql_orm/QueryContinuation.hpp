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
 *   - `rebindStdTmReferences`: Re-convert `std::tm`s to `MYSQL_TIME` for
 *     references that might have been updated in DSLs.
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
  static inline constexpr auto query_type{Query::query_type};

  template <typename... ContinuationArgs>
  constexpr QueryContinuation(MYSQL& mysql,
                              Query q,
                              Table const& t,
                              ContinuationArgs... args) noexcept
    : Continuation{
          mysql, std::move(q), t, std::forward<ContinuationArgs>(args)...}
  {
  }
  constexpr QueryContinuation(QueryContinuation const& b) = default;
  constexpr QueryContinuation(QueryContinuation&& b) noexcept = default;
  ~QueryContinuation() noexcept = default;

  constexpr QueryContinuation& operator=(QueryContinuation const& rhs) = default;
  constexpr QueryContinuation& operator=(QueryContinuation&& rhs) noexcept = default;

  template <typename T>
  constexpr auto operator()(T&& t) noexcept
  {
    return Continuation::operator()(std::forward<T>(t));
  }

  auto operator()()
  {
    return this->build().execute();
  }

  constexpr auto buildquery() const noexcept
  {
    return this->buildqueryCS();
  }

  static constexpr size_t getNbInputSlots() noexcept
  {
    return getNbInputSlotsImpl<QueryContinuation>(0);
  }

  static constexpr size_t getNbOutputSlots() noexcept
  {
    return Query::getNbOutputSlots();
  }

  template <std::size_t NBINDS>
  void bindInTo(InputBindArray<NBINDS>& binds) const noexcept
  {
    this->bindInToImpl(*this, binds, 0);
  }

  template <std::size_t NBINDS>
  void bindOutTo(model_type& model, OutputBindArray<NBINDS>& binds) const
  {
    this->query.bindOutTo(model, binds);
  }

  template <std::size_t NBINDS>
  void rebindStdTmReferences(InputBindArray<NBINDS>& ins) const noexcept
  {
    this->rebindStdTmReferencesImpl(*this, ins, 0);
  }

  template <std::size_t NBINDS>
  void finalizeBindings(model_type& model, OutputBindArray<NBINDS>& binds)
  {
    this->query.finalizeBindings(model, binds);
  }

  constexpr Statement<QueryContinuation, model_type> build() const noexcept
  {
    return Statement<QueryContinuation, model_type>{*this->mysql_handle, *this};
  }

private:
  template <typename Q>
  static constexpr auto getNbInputSlotsImpl(int) noexcept
      -> decltype(std::declval<Q>().Continuation::getNbInputSlots(), size_t())
  {
    return Continuation::getNbInputSlots();
  }
  template <typename Q>
  static constexpr size_t getNbInputSlotsImpl(long) noexcept
  {
    return Query::getNbInputSlots();
  }

  template <typename Q, std::size_t NBINDS>
  static constexpr auto bindInToImpl(Q const& q, InputBindArray<NBINDS>& binds, int) noexcept
      -> decltype(q.Continuation::bindInTo(binds), void())
  {
    q.Continuation::bindInTo(binds);
  }
  template <typename Q, std::size_t NBINDS>
  static constexpr void bindInToImpl(Q const& q, InputBindArray<NBINDS>& binds, long) noexcept
  {
    q.query.bindInTo(binds);
  }

  template <typename Q, std::size_t NBINDS>
  static constexpr auto rebindStdTmReferencesImpl(Q const& q,
                                        InputBindArray<NBINDS>& ins,
                                        int) noexcept
      -> decltype(q.Continuation::rebindStdTmReferences(ins), void())
  {
    q.Continuation::rebindStdTmReferences(ins);
  }
  template <typename Q, std::size_t NBINDS>
  static void rebindStdTmReferencesImpl(Q const& q,
                                        InputBindArray<NBINDS>& ins,
                                        long) noexcept
  {
    q.query.rebindStdTmReferences(ins);
  }
};

template <typename Query, typename Table, typename Continuation>
QueryContinuation(Query, Table, Continuation)
    ->QueryContinuation<Query, Table, Continuation>;
}

#endif /* !MYSQL_ORM_QUERYCONTINUATION_HPP_ */
