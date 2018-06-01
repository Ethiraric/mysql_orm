#ifndef MYSQL_ORM_COLUMNTAGS_HPP_
#define MYSQL_ORM_COLUMNTAGS_HPP_

#include <stdexcept>

namespace mysql_orm
{
enum class Tristate
{
  Undefined,
  On,
  Off
};

struct ColumnTags
{
  template <typename... TagList>
  constexpr ColumnTags(TagList... tags)
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
  template <typename Column>
  constexpr void operator()(Column& col)
  {
    if (col.nullable != Tristate::Undefined)
      throw std::runtime_error("Nullable specified multiple times");
    col.nullable = Tristate::On;
  }
};

struct NotNull
{
  template <typename Column>
  constexpr void operator()(Column& col)
  {
    if (col.nullable != Tristate::Undefined)
      throw std::runtime_error("Nullable specified multiple times");
    col.nullable = Tristate::Off;
  }
};

struct PrimaryKey
{
  template <typename Column>
  constexpr void operator()(Column& col)
  {
    if (col.primary_key)
      throw std::runtime_error("Primary key specified multiple times");
    col.primary_key = true;
  }
};

struct Autoincrement
{
  template <typename Column>
  constexpr void operator()(Column& col)
  {
    if (col.auto_increment)
      throw std::runtime_error("Autoincrement specified multiple times");
    col.auto_increment = true;
  }
};
}

#endif /* !MYSQL_ORM_COLUMNTAGS_HPP_ */
