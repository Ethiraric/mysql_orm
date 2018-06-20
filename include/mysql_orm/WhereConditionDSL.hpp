#ifndef MYSQL_ORM_WHERECONDITIONDSL_HPP_
#define MYSQL_ORM_WHERECONDITIONDSL_HPP_

#include <ostream>

#include <mysql/mysql.h>

#include <mysql_orm/StatementBinder.hpp>

namespace mysql_orm
{
enum class OperatorType
{
  Equals,
  NotEquals,
  GreaterThan,
  GreaterOrEquals,
  LessThan,
  LessOrEquals,
  And,
  Or
};

template <OperatorType type>
constexpr auto operatorTypeToString()
{
  if constexpr (type == OperatorType::Equals)
    return '=';
  else if constexpr (type == OperatorType::NotEquals)
    return "<>";
  else if constexpr (type == OperatorType::GreaterThan)
    return ">";
  else if constexpr (type == OperatorType::GreaterOrEquals)
    return ">=";
  else if constexpr (type == OperatorType::LessThan)
    return "<";
  else if constexpr (type == OperatorType::LessOrEquals)
    return "<=";
  else if constexpr (type == OperatorType::And)
    return " AND ";
  else if constexpr (type == OperatorType::Or)
    return " OR ";
}

template <typename T>
struct OperandWrapper
{
  template <typename Table>
  void appendToQuery(std::ostream& out, Table const& t) const
  {
    (void)(t);
    out << '?';
  }

  size_t getNbInputSlots() const noexcept
  {
    return 1;
  }

  void bindInTo(MYSQL_BIND* bindarray) const noexcept
  {
    StatementInBinder<T>::bind(this->value, bindarray);
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
    out << '?';
  }

  size_t getNbInputSlots() const noexcept
  {
    return 1;
  }

  void bindInTo(MYSQL_BIND* bindarray) const noexcept
  {
    StatementInBinder<T>::bind(this->value.get(), bindarray);
  }

  std::reference_wrapper<T> value;
};

template <typename T>
ref(T&)->ref<T>;

template <typename Lhs, typename Rhs, OperatorType type>
struct OperatorClosure
{
  template <typename Table>
  void appendToQuery(std::ostream& out, Table const& t) const
  {
    this->lhs.appendToQuery(out, t);
    out << operatorTypeToString<type>();
    this->rhs.appendToQuery(out, t);
  }

  size_t getNbInputSlots() const noexcept
  {
    return this->lhs.getNbInputSlots() + this->rhs.getNbInputSlots();
  }

  void bindInTo(MYSQL_BIND* bindarray) const noexcept
  {
    this->lhs.bindInTo(bindarray);
    this->rhs.bindInTo(bindarray + this->lhs.getNbInputSlots());
  }

#define MAKE_OPERATORS(op, optype)                                            \
  template <typename oLhs, typename oRhs, OperatorType otype>                 \
  auto operator op(OperatorClosure<oLhs, oRhs, otype>&& rhs_cond) const       \
  {                                                                           \
    return OperatorClosure<OperatorClosure<Lhs, Rhs, type>,                   \
                           OperatorClosure<oLhs, oRhs, otype>,                \
                           OperatorType::optype>{*this, std::move(rhs_cond)}; \
  }
  MAKE_OPERATORS(&&, And)
  MAKE_OPERATORS(||, Or)
#undef MAKE_OPERATORS

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

  size_t getNbInputSlots() const noexcept
  {
    return 0;
  }

  void bindInTo(MYSQL_BIND*) const noexcept
  {
  }

#define MAKE_OPERATORS(op, type)                                              \
  template <typename T>                                                       \
  auto operator op(T const& rhs)                                              \
  {                                                                           \
    return OperatorClosure<c, OperandWrapper<T>, OperatorType::type>{         \
        *this, OperandWrapper<T>{rhs}};                                       \
  }                                                                           \
                                                                              \
  template <auto other_attr>                                                  \
  auto operator op(c<other_attr> const& rhs)                                  \
  {                                                                           \
    return OperatorClosure<c, decltype(rhs), OperatorType::type>{*this, rhs}; \
  }                                                                           \
                                                                              \
  template <typename T>                                                       \
  auto operator op(ref<T> const& rhs)                                         \
  {                                                                           \
    return OperatorClosure<c, decltype(rhs), OperatorType::type>{*this, rhs}; \
  }

  MAKE_OPERATORS(==, Equals)
  MAKE_OPERATORS(!=, NotEquals)
  MAKE_OPERATORS(>, GreaterThan)
  MAKE_OPERATORS(>=, GreaterOrEquals)
  MAKE_OPERATORS(<, LessThan)
  MAKE_OPERATORS(<=, LessOrEquals)
#undef MAKE_OPERATORS
};
}

#endif /* !MYSQL_ORM_WHERECONDITIONDSL_HPP_ */
