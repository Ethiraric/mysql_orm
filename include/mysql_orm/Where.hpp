#ifndef MYSQL_ORM_WHERE_HPP_
#define MYSQL_ORM_WHERE_HPP_

#include <sstream>
#include <utility>

#include <mysql_orm/Limit.hpp>
#include <mysql_orm/Statement.hpp>

namespace mysql_orm
{
template <typename T>
struct OperandWrapper
{
  template <typename Table>
  void appendToQuery(std::ostream& out, Table const& t) const
  {
    (void)(t);
    out << value;
  }

  T value;
};

template <typename Lhs, typename Rhs>
struct EqualsClosure
{
  using compare_tag = int;

  template <typename Table>
  void appendToQuery(std::ostream& out, Table const& t) const
  {
    this->lhs.appendToQuery(out, t);
    out << '=';
    this->rhs.appendToQuery(out, t);
  }

  Lhs lhs;
  Rhs rhs;
};

template <typename Lhs, typename Rhs>
EqualsClosure(Lhs, Rhs)->EqualsClosure<Lhs, Rhs>;

template <auto attr>
struct c
{
  static inline constexpr auto attribute = attr;

  template <typename Table>
  void appendToQuery(std::ostream& out, Table const& t) const
  {
    out << '`' << t.template getColumn<attr>().getName() << '`';
  }

  template <typename T>
  auto operator==(T const& rhs)
  {
    return EqualsClosure{*this, OperandWrapper<T>{rhs}};
  }

  template <auto other_attr>
  auto operator==(c<other_attr> const& rhs)
  {
    return EqualsClosure{*this, rhs};
  }
};

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
