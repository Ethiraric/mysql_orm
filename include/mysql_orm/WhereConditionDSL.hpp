#ifndef MYSQL_ORM_WHERECONDITIONDSL_HPP_
#define MYSQL_ORM_WHERECONDITIONDSL_HPP_

#include <ostream>

namespace mysql_orm
{
enum class OperatorType
{
  Equals,
  NotEquals
};

template <OperatorType type>
constexpr auto operatorTypeToString()
{
  if constexpr (type == OperatorType::Equals)
    return '=';
  else if constexpr (type == OperatorType::NotEquals)
    return "<>";
}

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

template <typename Lhs, typename Rhs, OperatorType type>
struct OperatorClosure
{
  using compare_tag = int;

  template <typename Table>
  void appendToQuery(std::ostream& out, Table const& t) const
  {
    this->lhs.appendToQuery(out, t);
    out << operatorTypeToString<type>();
    this->rhs.appendToQuery(out, t);
  }

  Lhs lhs;
  Rhs rhs;
};

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
    return OperatorClosure<c, OperandWrapper<T>, OperatorType::Equals>{
        *this, OperandWrapper<T>{rhs}};
  }

  template <auto other_attr>
  auto operator==(c<other_attr> const& rhs)
  {
    return OperatorClosure<c, decltype(rhs), OperatorType::Equals>{*this, rhs};
  }

  template <typename T>
  auto operator==(ref<T> const& rhs)
  {
    return OperatorClosure<c, decltype(rhs), OperatorType::Equals>{*this, rhs};
  }
};
}

#endif /* !MYSQL_ORM_WHERECONDITIONDSL_HPP_ */
