#ifndef MYSQL_ORM_COLUMN_HPP_
#define MYSQL_ORM_COLUMN_HPP_

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

#include <mysql_orm/ColumnTags.hpp>
#include <mysql_orm/TypeTraits.hpp>

namespace mysql_orm
{
using id_t = uint32_t;

template <typename Field>
char const* getFieldSQLType();

template <>
inline char const* getFieldSQLType<std::string>()
{
  return "TEXT";
}

template <>
inline char const* getFieldSQLType<int32_t>()
{
  return "INTEGER";
}

template <>
inline char const* getFieldSQLType<uint32_t>()
{
  return "INTEGER UNSIGNED";
}

template <typename Model, typename Field, Field Model::*attr>
class Column
{
public:
  using model_type = Model;
  using field_type = Field;
  static inline constexpr auto attribute = attr;

  Column(std::string name, ColumnTags t = ColumnTags{}) noexcept
    : column_name{std::move(name)}, tags{t}
  {
    if (this->tags.nullable == Tristate::Undefined)
      this->tags.nullable = Tristate::Off;
  }
  Column(Column const& b) = default;
  Column(Column&& b) noexcept = default;
  ~Column() noexcept = default;

  Column& operator=(Column const& rhs) = default;
  Column& operator=(Column&& rhs) noexcept = default;

  std::string getSchema() const
  {
    auto const tagstr = this->tags.toString();
    auto schema = std::string{};

    schema = '`' + this->column_name + '`' + ' ' + getFieldSQLType<Field>();
    if (!tagstr.empty())
      schema += ' ' + tagstr;
    return schema;
  }

private:
  std::string column_name;
  ColumnTags tags;
};

template <typename Model, typename Field, std::optional<Field> Model::*attr>
class Column<Model, std::optional<Field>, attr>
{
public:
  using model_type = Model;
  using field_type = Field;
  static inline constexpr auto attribute = attr;

  explicit Column(std::string name, ColumnTags t = ColumnTags{}) noexcept
    : column_name{std::move(name)}, tags{t}
  {
    if (this->tags.nullable == Tristate::Undefined)
      this->tags.nullable = Tristate::On;
  }
  Column(Column const& b) = default;
  Column(Column&& b) noexcept = default;
  ~Column() noexcept = default;

  Column& operator=(Column const& rhs) = default;
  Column& operator=(Column&& rhs) noexcept = default;

  std::string getSchema() const
  {
    auto const tagstr = this->tags.toString();
    auto schema = std::string{};

    schema = '`' + this->column_name + '`' + ' ' + getFieldSQLType<Field>();
    if (!tagstr.empty())
      schema += ' ' + tagstr;
    return schema;
  }

private:
  std::string const column_name;
  ColumnTags tags;
};

template <auto AttributePtr, typename... Tags>
auto make_column(std::string name, Tags... tagattributes)
{
  using class_t =
      typename AttributePtrDissector<decltype(AttributePtr)>::class_t;
  using attribute_t =
      typename AttributePtrDissector<decltype(AttributePtr)>::attribute_t;
  constexpr auto tags = ColumnTags{tagattributes...};
  return Column<class_t, attribute_t, AttributePtr>{std::move(name), tags};
}
}

#endif /* !MYSQL_ORM_COLUMN_HPP_ */
