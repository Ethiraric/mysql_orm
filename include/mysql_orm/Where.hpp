#ifndef MYSQL_ORM_WHERE_HPP_
#define MYSQL_ORM_WHERE_HPP_

#include <sstream>
#include <utility>

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
  Where(Condition&& c) noexcept : condition{std::move(c)}
  {
  }

  template <typename Table>
  void appendToQuery(std::ostream& out, Table const& t) const
  {
    this->condition.appendToQuery(out, t);
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

  WhereQueryImpl(MYSQL& mysql, Query q, Table const& t, Condition&& c) noexcept
    : mysql_handle{&mysql},
      query{std::move(q)},
      table{t},
      condition{std::move(c)}
  {
  }
  WhereQueryImpl(WhereQueryImpl const& b) noexcept = default;
  WhereQueryImpl(WhereQueryImpl&& b) noexcept = default;
  ~WhereQueryImpl() noexcept = default;

  WhereQueryImpl& operator=(WhereQueryImpl const& rhs) noexcept = default;
  WhereQueryImpl& operator=(WhereQueryImpl&& rhs) noexcept = default;

  std::stringstream buildqueryss() const
  {
    auto ss = this->query.buildqueryss();
    ss << " WHERE ";
    this->condition.appendToQuery(ss, this->table.get());
    return ss;
  }

  template <typename Limit>
  LimitQuery<WhereQueryImpl, Table, Limit> operator()(Limit limit)
  {
    return LimitQuery<WhereQueryImpl, Table, Limit>{
        *this->mysql_handle, *this, this->table.get(), std::move(limit)};
  }

  size_t getNbInputSlots() const noexcept
  {
    return this->query.getNbInputSlots() + this->condition.getNbInputSlots();
  }

  void bindInTo(InputBindArray& binds) const noexcept
  {
    this->query.bindInTo(binds);
    this->condition.bindInTo(binds, this->query.getNbInputSlots());
  }

  void rebindStdTmReferences(InputBindArray& ins) const noexcept
  {
    this->query.rebindStdTmReferences(ins);
    this->condition.rebindStdTmReferences(ins, this->query.getNbInputSlots());
  }

protected:
  // May not be nullptr. Can't use std::reference_wrapper since MYSQL is
  // incomplete.
  MYSQL* mysql_handle;
  Query query;
  std::reference_wrapper<Table const> table;

private:
  Condition condition;
};

template <typename Query, typename Table, typename Condition>
using WhereQuery =
    QueryContinuation<Query, Table, WhereQueryImpl<Query, Table, Condition>>;
}

#endif /* !MYSQL_ORM_WHERE_HPP_ */
