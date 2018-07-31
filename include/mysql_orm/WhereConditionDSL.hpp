#ifndef MYSQL_ORM_WHERECONDITIONDSL_HPP_
#define MYSQL_ORM_WHERECONDITIONDSL_HPP_

#include <chrono>
#include <ostream>

#include <CompileString/CompileString.hpp>
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
constexpr auto operatorTypeToString() noexcept
{
  if constexpr (type == OperatorType::Equals)
    return compile_string::CompileString{"="};
  else if constexpr (type == OperatorType::NotEquals)
    return compile_string::CompileString{"<>"};
  else if constexpr (type == OperatorType::GreaterThan)
    return compile_string::CompileString{">"};
  else if constexpr (type == OperatorType::GreaterOrEquals)
    return compile_string::CompileString{">="};
  else if constexpr (type == OperatorType::LessThan)
    return compile_string::CompileString{"<"};
  else if constexpr (type == OperatorType::LessOrEquals)
    return compile_string::CompileString{"<="};
  else if constexpr (type == OperatorType::And)
    return compile_string::CompileString{" AND "};
  else if constexpr (type == OperatorType::Or)
    return compile_string::CompileString{" OR "};
}

template <typename Lhs, typename Rhs>
struct Assignment;

template <typename Head, typename Tail>
struct AssignmentList
{
  template <std::size_t N>
  using CompileString = compile_string::CompileString<N>;

  template <typename Lhs, typename Rhs>
  auto operator,(Assignment<Lhs, Rhs> assignment)
  {
    return AssignmentList<Assignment<Lhs, Rhs>, AssignmentList<Head, Tail>>{
        std::move(assignment), *this};
  }

  template <std::size_t N, typename Table>
  auto appendToQuery(CompileString<N> const& query, Table const& t) const
  {
    return this->tail.appendToQuery(this->head.appendToQuery(query, t) + ", ",
                                    t);
  }

  static constexpr size_t getNbInputSlots() noexcept
  {
    return Head::getNbInputSlots() + Tail::getNbInputSlots();
  }

  template <std::size_t NBINDS>
  void bindInTo(InputBindArray<NBINDS>& binds, std::size_t idx) const
  {
    this->head.bindInTo(binds, idx);
    this->tail.bindInTo(binds, idx + head.getNbInputSlots());
  }

  template <std::size_t NBINDS>
  void rebindStdTmReferences(InputBindArray<NBINDS>&, std::size_t) const noexcept
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
  template <std::size_t N>
  using CompileString = compile_string::CompileString<N>;

  template <typename OLhs, typename ORhs>
  auto operator,(Assignment<OLhs, ORhs> other_assignment)
  {
    return AssignmentList{std::move(other_assignment), *this};
  }

  template <std::size_t N, typename Table>
  auto appendToQuery(CompileString<N> const& query, Table const& t) const
  {
    return this->rhs.appendToQuery(this->lhs.appendToQuery(query, t) + "=", t);
  }

  static constexpr size_t getNbInputSlots() noexcept
  {
    return Lhs::getNbInputSlots() + Rhs::getNbInputSlots();
  }

  template <std::size_t NBINDS>
  void bindInTo(InputBindArray<NBINDS>& binds, std::size_t idx) const
  {
    this->lhs.bindInTo(binds, idx);
    this->rhs.bindInTo(binds, idx + lhs.getNbInputSlots());
  }

  template <std::size_t NBINDS>
  void rebindStdTmReferences(InputBindArray<NBINDS>&, std::size_t) const noexcept
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
  template <std::size_t N>
  using CompileString = compile_string::CompileString<N>;

  template <std::size_t N, typename Table>
  auto appendToQuery(CompileString<N> const& query, Table const&) const
  {
    return query + "?";
  }

  static constexpr size_t getNbInputSlots() noexcept
  {
    return 1;
  }

  template <std::size_t NBINDS>
  void bindInTo(InputBindArray<NBINDS>& binds, std::size_t idx) const
  {
    binds.bind(idx, this->value);
  }

  template <std::size_t NBINDS>
  void rebindStdTmReferences(InputBindArray<NBINDS>&, std::size_t) const noexcept
  {
  }

  T value;
};

template <typename T>
struct ref
{
  template <std::size_t N>
  using CompileString = compile_string::CompileString<N>;

  template <std::size_t N, typename Table>
  auto appendToQuery(CompileString<N> const& query, Table const&) const
  {
    return query + "?";
  }

  static constexpr size_t getNbInputSlots() noexcept
  {
    return 1;
  }

  template <std::size_t NBINDS>
  void bindInTo(InputBindArray<NBINDS>& binds, std::size_t idx) const
  {
    binds.bind(idx, this->value.get());
  }

  template <std::size_t NBINDS>
  void rebindStdTmReferences(InputBindArray<NBINDS>& binds,
                             std::size_t idx) const
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
  template <std::size_t N>
  using CompileString = compile_string::CompileString<N>;

  template <std::size_t N, typename Table>
  auto appendToQuery(CompileString<N> const& query, Table const& t) const
  {
    return this->rhs.appendToQuery(
        this->lhs.appendToQuery(query, t) + operatorTypeToString<type>(), t);
  }

  static constexpr size_t getNbInputSlots() noexcept
  {
    return Lhs::getNbInputSlots() + Rhs::getNbInputSlots();
  }

  template <std::size_t NBINDS>
  void bindInTo(InputBindArray<NBINDS>& binds, std::size_t idx) const 
  {
    this->lhs.bindInTo(binds, idx);
    this->rhs.bindInTo(binds, idx + lhs.getNbInputSlots());
  }

  template <std::size_t NBINDS>
  void rebindStdTmReferences(InputBindArray<NBINDS>& binds, std::size_t idx) const
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
  template <std::size_t N>
  using CompileString = compile_string::CompileString<N>;

  static inline constexpr auto attribute = attr;

  template <std::size_t N, typename Table>
  auto appendToQuery(CompileString<N> const& query, Table const& t) const noexcept
  {
    return query + "`" + t.template getColumn<attr>().getName() + "`";
  }

  static constexpr size_t getNbInputSlots() noexcept
  {
    return 0;
  }

  template <std::size_t NBINDS>
  void bindInTo(InputBindArray<NBINDS>&, std::size_t) const noexcept
  {
  }

  template <std::size_t NBINDS>
  void rebindStdTmReferences(InputBindArray<NBINDS>&, std::size_t) const noexcept
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
