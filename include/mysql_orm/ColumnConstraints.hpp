#ifndef MYSQL_ORM_COLUMNCONSTRAINTS_HPP_
#define MYSQL_ORM_COLUMNCONSTRAINTS_HPP_

#include <stdexcept>
#include <type_traits>

#include <CompileString/CompileString.hpp>

#include <mysql_orm/Utils.hpp>
#include <mysql_orm/meta/Pack.hpp>

namespace mysql_orm
{
/** Either On, Off, or Undefined.
 *
 * A value of Undefined means the default behavior should be used.
 */
enum class Tristate
{
  Undefined,
  On,
  Off
};

template <typename... ConstraintsList>
struct ConstraintsAggregation
{
  constexpr ConstraintsAggregation()
  {
    if constexpr (sizeof...(ConstraintsList) > 0)
      this->apply<ConstraintsList...>();
  }

  template <typename Constraint, typename... Constraints>
  constexpr void apply()
  {
    Constraint::apply(*this);
    if constexpr (sizeof...(Constraints) > 0)
      this->apply<Constraints...>();
  }

  bool unique{false};
  Tristate nullable{Tristate::Undefined};
  bool primary_key{false};
  bool auto_increment{false};
};

/** Aggregation of the different column constraints.
 *
 * Each Constraint must have overloaded its operator(). Upon construction,
 * ColumnConstraints will invoke each of its arguments' operator(). Each of
 * them should edit the constraint(s) accordingly.
 *
 * One can create its own class editing more than one constraints to have a
 * custom behavior.
 */
template <typename... ConstraintsList>
struct ColumnConstraints
{
  constexpr ColumnConstraints() = default;
  constexpr ColumnConstraints(ColumnConstraints const& b) noexcept = default;
  constexpr ColumnConstraints(ColumnConstraints&& b) noexcept = default;

  static constexpr auto toString() noexcept
  {
    /** The following structures are used as index for iterating over a tuple
     * with another tuple... Looking for a better solution for now, but
     * manipulating CompileStrings is not as easy as I thought it would be.
     */
    // clang-format off
    struct T0 {};
    struct T1 {};
    struct T2 {};
    struct T3 {};
    // clang-format on

    auto iter_tuple = std::tuple<T0, T1, T2, T3>{};

    return tupleFoldl(
        [&](auto const& acc, auto iter) constexpr {
          auto tmpstr = [&]() {
            if constexpr (!std::is_same_v<
                              compile_string::CompileString<0> const&,
                              decltype(acc)>)
              return acc + " ";
            else
              return acc;
          }();

          if constexpr (std::is_same_v<decltype(iter), T0> &&
                        unique())
            return tmpstr + "UNIQUE";
          else if constexpr (std::is_same_v<decltype(iter), T1> &&
                             nullable() == Tristate::Off)
            return tmpstr + "NOT NULL";
          else if constexpr (std::is_same_v<decltype(iter), T2> &&
                             primary_key())
            return tmpstr + "PRIMARY KEY";
          else if constexpr (std::is_same_v<decltype(iter), T3> &&
                             auto_increment())
            return tmpstr + "AUTO_INCREMENT";
          else
            return acc;
        },
        compile_string::CompileString<0>{""},
        iter_tuple);
  }

  static constexpr bool unique() noexcept
  {
    return build().unique;
  }

  static constexpr Tristate nullable() noexcept
  {
    return build().nullable;
  }

  static constexpr bool primary_key() noexcept
  {
    return build().primary_key;
  }

  static constexpr bool auto_increment() noexcept
  {
    return build().auto_increment;
  }

private:
  static constexpr auto build() noexcept
  {
    return ConstraintsAggregation<ConstraintsList...>{};
  }
};

template <typename... Constraints>
constexpr auto columnConstraintsFromPack(meta::Pack<Constraints...>)
{
  return ColumnConstraints<Constraints...>{};
}

struct Nullable
{
  template <typename Constraints>
  static constexpr void apply(Constraints& tags)
  {
    if (tags.nullable != Tristate::Undefined)
      throw std::runtime_error("Nullable specified multiple times");
    tags.nullable = Tristate::On;
  }
};

struct NotNull
{
  template <typename Constraints>
  static constexpr void apply(Constraints& tags)
  {
    if (tags.nullable != Tristate::Undefined)
      throw std::runtime_error("Nullable specified multiple times");
    tags.nullable = Tristate::Off;
  }
};

struct PrimaryKey
{
  template <typename Constraints>
  static constexpr void apply(Constraints& tags)
  {
    if (tags.primary_key)
      throw std::runtime_error("Primary key specified multiple times");
    tags.primary_key = true;
  }
};

struct Autoincrement
{
  template <typename Constraints>
  static constexpr void apply(Constraints& tags)
  {
    if (tags.auto_increment)
      throw std::runtime_error("Autoincrement specified multiple times");
    tags.auto_increment = true;
  }
};

struct Unique
{
  template <typename Constraints>
  static constexpr void apply(Constraints& tags)
  {
    if (tags.unique)
      throw std::runtime_error("Unique specified multiple times");
    tags.unique = true;
  }
};
}

#endif /* !MYSQL_ORM_COLUMNCONSTRAINTS_HPP_ */
