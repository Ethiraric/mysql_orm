#ifndef MYSQL_ORM_WHERE_HPP_
#define MYSQL_ORM_WHERE_HPP_

#include <sstream>
#include <utility>

#include <CompileString/CompileString.hpp>

#include <mysql_orm/Limit.hpp>
#include <mysql_orm/Statement.hpp>
#include <mysql_orm/WhereConditionDSL.hpp>

namespace mysql_orm
{
/** Where clause arguments.
 *
 * This class is used as an argument to a `Select`'s or `Update`'s `operator()`.
 * The constructor takes as argument the condition to apply to the Where query.
 * See the `c` and `ref` functions.
 *
 * This class is not the actual query but a class that serves as a tag for the
 * other query classes.
 * The query class is `WhereQueryImpl`.
 */
template <typename Condition>
struct Where
{
public:
  constexpr Where(Condition&& c) noexcept : condition{std::move(c)}
  {
  }

  template <typename Table, std::size_t T>
  constexpr auto appendToQuery(compile_string::CompileString<T> const& query,
                               Table const& t) const noexcept
  {
    return this->condition.appendToQuery(query, t);
  }

  Condition condition;
};

/** A Where query.
 *
 * The class continues a `Select` or `Update` query.
 * Takes a condition as parameter, which must be an `OperatorClosure`.
 *
 * `buildquery` returns the SQL query as a std::string.
 * `build` returns a `Statement`, which can later be `execute()`d.
 *
 * The `operator()` can be used to continue the query (Limit).
 *
 * TODO(ethiraric): Check that all columns from the conditions refer to the
 * model.
 */
template <typename Query, typename Table, typename Condition>
class WhereQueryImpl
{
public:
  using model_type = typename Query::model_type;
  static inline constexpr auto query_type{Query::query_type};

  constexpr WhereQueryImpl(MYSQL& mysql, Query q, Table const& t, Condition&& c) noexcept
    : mysql_handle{&mysql},
      query{std::move(q)},
      table{&t},
      condition{std::move(c)}
  {
  }
  constexpr WhereQueryImpl(WhereQueryImpl const& b) = default;
  constexpr WhereQueryImpl(WhereQueryImpl&& b) noexcept = default;
  ~WhereQueryImpl() noexcept = default;

  constexpr WhereQueryImpl& operator=(WhereQueryImpl const& rhs) = default;
  constexpr WhereQueryImpl& operator=(WhereQueryImpl&& rhs) noexcept = default;

  constexpr auto buildqueryCS() const noexcept
  {
    return this->condition.appendToQuery(this->query.buildqueryCS() + " WHERE ",
                                         *this->table);
  }

  template <typename Limit>
  constexpr auto operator()(Limit limit)
  {
    using ContinuationType = QueryContinuation<Query, Table, WhereQueryImpl>;
    return LimitQuery<ContinuationType, Table, Limit>{
        *this->mysql_handle,
        static_cast<ContinuationType&>(*this),
        *this->table,
        std::move(limit)};
  }

  static constexpr size_t getNbInputSlots() noexcept
  {
    return Query::getNbInputSlots() + Condition::getNbInputSlots();
  }

  template <std::size_t NBINDS>
  void bindInTo(InputBindArray<NBINDS>& binds) const noexcept
  {
    this->query.bindInTo(binds);
    this->condition.bindInTo(binds, this->query.getNbInputSlots());
  }

  template <std::size_t NBINDS>
  void rebindStdTmReferences(InputBindArray<NBINDS>& ins) const noexcept
  {
    this->query.rebindStdTmReferences(ins);
    this->condition.rebindStdTmReferences(ins, this->query.getNbInputSlots());
  }

protected:
  // May not be nullptr. Can't use std::reference_wrapper since MYSQL is
  // incomplete.
  MYSQL* mysql_handle;
  Query query;
  Table const* table;

private:
  Condition condition;
};

template <typename Query, typename Table, typename Condition>
using WhereQuery =
    QueryContinuation<Query, Table, WhereQueryImpl<Query, Table, Condition>>;
}

#endif /* !MYSQL_ORM_WHERE_HPP_ */
