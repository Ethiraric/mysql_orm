#ifndef MYSQL_ORM_COLUMN_HPP_
#define MYSQL_ORM_COLUMN_HPP_

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

#include <mysql_orm/TypeTraits.hpp>

namespace mysql_orm
{
using id_t = uint32_t;

template <typename Field>
char const* getFieldSQLType();

template <>
char const* getFieldSQLType<std::string>()
{
  return "TEXT";
}

template <>
char const* getFieldSQLType<int32_t>()
{
  return "INTEGER";
}

template <>
char const* getFieldSQLType<uint32_t>()
{
  return "INTEGER UNSIGNED";
}

template <typename Model, typename Field, Field Model::*attr>
class Column
{
public:
  explicit Column(std::string name) noexcept : column_name{std::move(name)}
  {
  }
  Column(Column const& b) noexcept = default;
  Column(Column&& b) noexcept = default;
  ~Column() noexcept = default;

  Column& operator=(Column const& rhs) noexcept = default;
  Column& operator=(Column&& rhs) noexcept = default;

  std::string getSchema() const
  {
    return '`' + this->column_name + '`' + ' ' + getFieldSQLType<Field>() +
           " NOT NULL";
  }

private:
  std::string column_name;
};

template <typename Model, typename Field, std::optional<Field> Model::*attr>
class Column<Model, std::optional<Field>, attr>
{
public:
  explicit Column(std::string name) noexcept : column_name{std::move(name)}
  {
  }
  Column(Column const& b) noexcept = default;
  Column(Column&& b) noexcept = default;
  ~Column() noexcept = default;

  Column& operator=(Column const& rhs) noexcept = default;
  Column& operator=(Column&& rhs) noexcept = default;

  std::string getSchema() const
  {
    return '`' + this->column_name + '`' + ' ' + getFieldSQLType<Field>();
  }

private:
  std::string column_name;
};

template <auto AttributePtr>
auto make_column(std::string name)
{
  using class_t =
      typename AttributePtrDissector<decltype(AttributePtr)>::class_t;
  using attribute_t =
      typename AttributePtrDissector<decltype(AttributePtr)>::attribute_t;
  return Column<class_t, attribute_t, AttributePtr>{std::move(name)};
}
}

#endif /* !MYSQL_ORM_COLUMN_HPP_ */
