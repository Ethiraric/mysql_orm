#ifndef MYSQL_ORM_COLUMNCONSTRAINTS_HPP_
#define MYSQL_ORM_COLUMNCONSTRAINTS_HPP_

#include <stdexcept>

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

/** Aggregation of the different column constraints.
 *
 * Each Tag must have overloaded its operator(). Upon construction,
 * ColumnConstraints will invoke each of its arguments' operator(). Each of
 * them should edit the constraint(s) accordingly.
 *
 * One can create its own class editing more than one constraints to have a
 * custom behavior.
 */
struct ColumnConstraints
{
  template <typename... TagList>
  constexpr ColumnConstraints(TagList... tags)
  {
    this->apply(tags...);
  }

  std::string toString() const
  {
    auto ret = std::string{};
    auto add_word = [](std::string& dst, char const* app) {
      if (!dst.empty())
        dst += ' ';
      dst += app;
    };

    if (this->nullable == Tristate::Off)
      add_word(ret, "NOT NULL");
    if (this->primary_key)
      add_word(ret, "PRIMARY KEY");
    if (this->auto_increment)
      add_word(ret, "AUTO_INCREMENT");
    return ret;
  }

  Tristate nullable{Tristate::Undefined};
  bool primary_key{false};
  bool auto_increment{false};

private:
  constexpr void apply()
  {
  }

  template <typename Tag, typename... TagList>
  constexpr void apply(Tag t, TagList... tags)
  {
    t(*this);
    this->apply(tags...);
  }
};

struct Nullable
{
  template <typename Constraints>
  constexpr void operator()(Constraints& tags)
  {
    if (tags.nullable != Tristate::Undefined)
      throw std::runtime_error("Nullable specified multiple times");
    tags.nullable = Tristate::On;
  }
};

struct NotNull
{
  template <typename Constraints>
  constexpr void operator()(Constraints& tags)
  {
    if (tags.nullable != Tristate::Undefined)
      throw std::runtime_error("Nullable specified multiple times");
    tags.nullable = Tristate::Off;
  }
};

struct PrimaryKey
{
  template <typename Constraints>
  constexpr void operator()(Constraints& tags)
  {
    if (tags.primary_key)
      throw std::runtime_error("Primary key specified multiple times");
    tags.primary_key = true;
  }
};

struct Autoincrement
{
  template <typename Constraints>
  constexpr void operator()(Constraints& tags)
  {
    if (tags.auto_increment)
      throw std::runtime_error("Autoincrement specified multiple times");
    tags.auto_increment = true;
  }
};
}

#endif /* !MYSQL_ORM_COLUMNCONSTRAINTS_HPP_ */
