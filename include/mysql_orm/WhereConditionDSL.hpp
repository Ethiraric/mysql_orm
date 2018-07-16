#ifndef MYSQL_ORM_WHERECONDITIONDSL_HPP_
#define MYSQL_ORM_WHERECONDITIONDSL_HPP_

#include <chrono>
#include <ostream>

#include <mysql/mysql.h>

#include <mysql_orm/BindArray.hpp>

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

template <typename Lhs, typename Rhs>
struct Assignment;

template <typename Head, typename Tail>
struct AssignmentList
{
  template <typename Lhs, typename Rhs>
  auto operator,(Assignment<Lhs, Rhs> assignment)
  {
    return AssignmentList<Assignment<Lhs, Rhs>, AssignmentList<Head, Tail>>{
        std::move(assignment), *this};
  }

  template <typename Table>
  void appendToQuery(std::ostream& out, Table const& t) const
  {
    this->head.appendToQuery(out, t);
    out << ',' << ' ';
    this->tail.appendToQuery(out, t);
  }

  size_t getNbInputSlots() const noexcept
  {
    return this->head.getNbInputSlots() + this->tail.getNbInputSlots();
  }

  void bindInTo(InputBindArray& binds, std::size_t idx) const noexcept
  {
    this->head.bindInTo(binds, idx);
    this->tail.bindInTo(binds, idx + head.getNbInputSlots());
  }

  void rebindStdTmReferences(InputBindArray&, std::size_t) const noexcept
  {
  }

  Head head;
  Tail tail;
};

template <typename Head, typename Tail>
AssignmentList(Head, Tail)->AssignmentList<Head, Tail>;

template <typename Lhs, typename Rhs>
struct Assignment
{
  template <typename OLhs, typename ORhs>
  auto operator,(Assignment<OLhs, ORhs> other_assignment)
  {
    return AssignmentList{std::move(other_assignment), *this};
  }

  template <typename Table>
  void appendToQuery(std::ostream& out, Table const& t) const
  {
    this->lhs.appendToQuery(out, t);
    out << '=';
    this->rhs.appendToQuery(out, t);
  }

  size_t getNbInputSlots() const noexcept
  {
    return this->lhs.getNbInputSlots() + this->rhs.getNbInputSlots();
  }

  void bindInTo(InputBindArray& binds, std::size_t idx) const noexcept
  {
    this->lhs.bindInTo(binds, idx);
    this->rhs.bindInTo(binds, idx + lhs.getNbInputSlots());
  }

  void rebindStdTmReferences(InputBindArray&, std::size_t) const noexcept
  {
  }

  Lhs lhs;
  Rhs rhs;
};

template <typename Lhs, typename Rhs>
Assignment(Lhs, Rhs)->Assignment<Lhs, Rhs>;

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

  void bindInTo(InputBindArray& binds, std::size_t idx) const noexcept
  {
    binds.bind(idx, this->value);
  }

  void rebindStdTmReferences(InputBindArray&, std::size_t) const noexcept
  {
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

  void bindInTo(InputBindArray& binds, std::size_t idx) const noexcept
  {
    binds.bind(idx, this->value.get());
  }

  void rebindStdTmReferences(InputBindArray& binds, std::size_t idx) const
      noexcept
  {
    binds.bind(idx, this->value.get());
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

  void bindInTo(InputBindArray& binds, std::size_t idx) const noexcept
  {
    this->lhs.bindInTo(binds, idx);
    this->rhs.bindInTo(binds, idx + lhs.getNbInputSlots());
  }

  void rebindStdTmReferences(InputBindArray& binds, std::size_t idx) const
      noexcept
  {
    this->lhs.rebindStdTmReferences(binds, idx);
    this->rhs.rebindStdTmReferences(binds, idx + lhs.getNbInputSlots());
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

  void bindInTo(InputBindArray&, std::size_t) const noexcept
  {
  }

  void rebindStdTmReferences(InputBindArray&, std::size_t) const noexcept
  {
  }

  template <typename T>
  auto operator=(T rhs) const
  {
    return Assignment{*this, OperandWrapper<T>{std::move(rhs)}};
  }

  template <typename T, size_t N>
  auto operator=(T const (&rhs)[N]) const
  {
    return Assignment{*this, OperandWrapper<T const&>{rhs}};
  }

  template <auto other_attr>
  auto operator=(c<other_attr> rhs) const
  {
    return Assignment{*this, rhs};
  }

  template <typename T>
  auto operator=(ref<T> rhs) const
  {
    return Assignment{*this, std::move(rhs)};
  }

#define MAKE_OPERATORS(op, type)                                              \
  template <typename T>                                                       \
  auto operator op(T const& rhs) const                                        \
  {                                                                           \
    return OperatorClosure<c, OperandWrapper<T>, OperatorType::type>{         \
        *this, OperandWrapper<T>{rhs}};                                       \
  }                                                                           \
                                                                              \
  template <typename T, size_t N>                                             \
  auto operator op(T const(&rhs)[N]) const                                    \
  {                                                                           \
    return OperatorClosure<c, OperandWrapper<T const*>, OperatorType::type>{  \
        *this, OperandWrapper<T const*>{rhs}};                                \
  }                                                                           \
                                                                              \
  template <auto other_attr>                                                  \
  auto operator op(c<other_attr> const& rhs) const                            \
  {                                                                           \
    return OperatorClosure<c, decltype(rhs), OperatorType::type>{*this, rhs}; \
  }                                                                           \
                                                                              \
  template <typename T>                                                       \
  auto operator op(ref<T> const& rhs) const                                   \
  {                                                                           \
    return OperatorClosure<c, ref<T>, OperatorType::type>{*this, rhs};        \
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
