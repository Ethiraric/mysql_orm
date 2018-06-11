#ifndef MYSQL_ORM_WHERE_HPP_
#define MYSQL_ORM_WHERE_HPP_

#include <sstream>
#include <utility>

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
class WhereQuery
{
public:
  using model_type = typename Query::model_type;

  WhereQuery(MYSQL& mysql, Query q, Table const& t, Condition&& c) noexcept
    : mysql_handle{&mysql},
      query{std::move(q)},
      table{t},
      condition{std::move(c)}
  {
  }
  WhereQuery(WhereQuery const& b) noexcept = default;
  WhereQuery(WhereQuery&& b) noexcept = default;
  ~WhereQuery() noexcept = default;

  WhereQuery& operator=(WhereQuery const& rhs) noexcept = default;
  WhereQuery& operator=(WhereQuery&& rhs) noexcept = default;

  std::stringstream buildqueryss() const
  {
    auto ss = this->query.buildqueryss();
    ss << " WHERE ";
    this->condition.appendToQuery(ss, this->table.get());
    return ss;
  }

  std::string buildquery() const
  {
    return this->buildqueryss().str();
  }

  Statement<model_type> build() const
  {
    return Statement<model_type>{
        *this->mysql_handle, this->buildquery(), this->getNbOutputSlots()};
  }

  size_t getNbOutputSlots() const noexcept
  {
    return this->query.getNbOutputSlots();
  }

  void bindOutTo(model_type& model, std::vector<MYSQL_BIND>& out_binds)
  {
    this->query.bindOutTo(model, out_binds);
  }

private:
  // May not be nullptr. Can't use std::reference_wrapper since MYSQL is
  // incomplete.
  MYSQL* mysql_handle;
  Query query;
  std::reference_wrapper<Table const> table;
  Condition condition;
};

template <typename Query, typename Table, typename Condition>
WhereQuery(Query, Table, Condition)->WhereQuery<Query, Table, Condition>;
}

#endif /* !MYSQL_ORM_WHERE_HPP_ */
