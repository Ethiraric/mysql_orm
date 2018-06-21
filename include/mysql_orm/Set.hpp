#ifndef MYSQL_ORM_SET_HPP_
#define MYSQL_ORM_SET_HPP_

#include <sstream>

#include <mysql_orm/QueryContinuation.hpp>
#include <mysql_orm/Where.hpp>
#include <mysql_orm/WhereConditionDSL.hpp>

namespace mysql_orm
{
template <typename Assignments>
struct Set
{
  Set(Assignments&& a) : assignments{a}
  {
  }

  Assignments assignments;
};

template <typename Query, typename Table, typename Assignments>
struct SetQueryImpl
{
public:
  using model_type = typename Query::model_type;
  static inline constexpr auto is_select_query{Query::is_select_query};

  SetQueryImpl(MYSQL& mysql, Query q, Table const& t, Assignments&& a) noexcept
    : mysql_handle{&mysql},
      query{std::move(q)},
      table{t},
      assignments{std::move(a)}
  {
  }
  SetQueryImpl(SetQueryImpl const& b) noexcept = default;
  SetQueryImpl(SetQueryImpl&& b) noexcept = default;
  ~SetQueryImpl() noexcept = default;

  SetQueryImpl& operator=(SetQueryImpl const& rhs) noexcept = default;
  SetQueryImpl& operator=(SetQueryImpl&& rhs) noexcept = default;

  template <typename Condition>
  WhereQuery<SetQueryImpl, Table, Condition> operator()(Where<Condition> where)
  {
    return WhereQuery<SetQueryImpl, Table, Condition>{
        *this->mysql_handle,
        *this,
        this->table.get(),
        std::move(where.condition)};
  }

  std::stringstream buildqueryss() const
  {
    auto ss = this->query.buildqueryss();
    ss << " SET ";
    this->assignments.appendToQuery(ss, this->table.get());
    return ss;
  }

  size_t getNbInputSlots() const noexcept
  {
    return this->query.getNbInputSlots() + this->assignments.getNbInputSlots();
  }

  void bindInTo(MYSQL_BIND* bindarray) const noexcept
  {
    auto* firstbind = bindarray + this->query.getNbInputSlots();
    this->assignments.bindInTo(firstbind);
    this->query.bindInTo(bindarray);
  }

protected:
  // May not be nullptr. Can't use std::reference_wrapper since MYSQL is
  // incomplete.
  MYSQL* mysql_handle;
  Query query;
  std::reference_wrapper<Table const> table;

private:
  Assignments assignments;
};

template <typename Query, typename Table, typename Condition>
using SetQuery =
    QueryContinuation<Query, Table, SetQueryImpl<Query, Table, Condition>>;
}

#endif /* !MYSQL_ORM_SET_HPP_ */
