#ifndef MYSQL_ORM_WHERE_HPP_
#define MYSQL_ORM_WHERE_HPP_

#include <sstream>
#include <utility>

#include <mysql_orm/Limit.hpp>
#include <mysql_orm/Statement.hpp>
#include <mysql_orm/WhereConditionDSL.hpp>

namespace mysql_orm
{
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

template <typename Query, typename Table, typename Condition>
class WhereQueryImpl
{
public:
  using model_type = typename Query::model_type;
  static inline constexpr auto is_select_query{Query::is_select_query};

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

  void bindInTo(MYSQL_BIND* bindarray) const noexcept
  {
    auto* firstbind = bindarray + this->query.getNbInputSlots();
    this->condition.bindInTo(firstbind);
    this->query.bindInTo(bindarray);
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
