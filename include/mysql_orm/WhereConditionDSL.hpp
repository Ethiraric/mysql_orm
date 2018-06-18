#ifndef MYSQL_ORM_WHERECONDITIONDSL_HPP_
#define MYSQL_ORM_WHERECONDITIONDSL_HPP_

#include <ostream>

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

template <typename T>
struct ref
{
  template <typename Table>
  void appendToQuery(std::ostream& out, Table const& t) const
  {
    (void)(t);
    out << value.get();
  }

  std::reference_wrapper<T> value;
};

template <typename T>
ref(T&)->ref<T>;

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

  template <typename T>
  auto operator==(ref<T> const& rhs)
  {
    return EqualsClosure{*this, rhs};
  }
};
}

#endif /* !MYSQL_ORM_WHERECONDITIONDSL_HPP_ */
