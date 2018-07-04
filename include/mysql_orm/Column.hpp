#ifndef MYSQL_ORM_COLUMN_HPP_
#define MYSQL_ORM_COLUMN_HPP_

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

#include <mysql_orm/ColumnConstraints.hpp>
#include <mysql_orm/meta/AttributePtrDissector.hpp>
#include <mysql_orm/meta/IsOptional.hpp>
#include <mysql_orm/meta/LiftOptional.hpp>

namespace mysql_orm
{
using id_t = uint32_t;

/** Returns a C string with the SQL type of the given field.
 *
 * The function errors when the type is unsupported.
 */
template <typename Field>
constexpr char const* getFieldSQLType()
{
  if constexpr (std::is_same_v<Field, std::string> ||
                std::is_same_v<Field, char*> ||
                std::is_same_v<Field, char const*>)
    return "TEXT";
  else if constexpr (std::is_integral_v<Field>)
  {
    if constexpr (std::is_same_v<Field, bool>)
      return "BOOLEAN";
    else if constexpr (std::is_same_v<Field, int8_t>)
      return "TINYINT";
    else if constexpr (std::is_same_v<Field, uint8_t>)
      return "TINYINT UNSIGNED";
    else if constexpr (std::is_same_v<Field, int16_t>)
      return "SMALLINT";
    else if constexpr (std::is_same_v<Field, uint16_t>)
      return "SMALLINT UNSIGNED";
    else if constexpr (std::is_same_v<Field, int32_t>)
      return "INTEGER";
    else if constexpr (std::is_same_v<Field, uint32_t>)
      return "INTEGER UNSIGNED";
    else if constexpr (std::is_same_v<Field, int64_t>)
      return "BIGINT";
    else if constexpr (std::is_same_v<Field, uint64_t>)
      return "BIGINT UNSIGNED";
  }
  else if constexpr (std::is_same_v<Field, std::tm>)
    return "DATETIME";
}

/** A Column in a table.
 *
 * The column can be constructed with a name and tags representing the
 * constraints. Users should not manipulate this class directly.
 * Fields are not nullable by default.
 *
 * We call `Model` the class from which the attribute is from.
 * We call `Field` the type of the attribute in the class.
 *
 * The class defines two member types:
 *   - `model_type`: Alias to the `Model` template argument.
 *   - `field_type`: Alias to the `Field` template argument.
 *
 * The class defines one static member:
 *   - `attribute`: Alias to the `attr` template argument (pointer to member).
 *
 * TODO(ethiraric): This class might be `constexpr`'d?
 */
template <typename Model, typename Field, Field Model::*attr>
class Column
{
public:
  using model_type = Model;
  using field_type = Field;
  using lifted_field_type = meta::LiftOptional_t<Field>;
  static inline constexpr auto attribute = attr;
  static inline constexpr auto is_optional = meta::IsOptional_v<Field>;

  Column(std::string name, ColumnConstraints t = ColumnConstraints{}) noexcept
    : column_name{std::move(name)}, tags{t}
  {
    if (this->tags.nullable == Tristate::Undefined)
      this->tags.nullable = is_optional ? Tristate::On : Tristate::Off;
  }
  Column(Column const& b) = default;
  Column(Column&& b) noexcept = default;
  ~Column() noexcept = default;

  Column& operator=(Column const& rhs) = default;
  Column& operator=(Column&& rhs) noexcept = default;

  /** Returns the part of the create statement associated with the column.
   */
  std::string getSchema() const
  {
    auto const tagstr = this->tags.toString();
    auto schema = std::string{};

    schema = '`' + this->column_name + '`' + ' ' +
             getFieldSQLType<lifted_field_type>();
    if (!tagstr.empty())
      schema += ' ' + tagstr;
    return schema;
  }

  std::string const& getName() const noexcept
  {
    return this->column_name;
  }

private:
  std::string column_name;
  ColumnConstraints tags;
};

/** Helper function to create a column.
 *
 * Used as `make_column<&Model::Field>("column_name", Autoincrement{},
 * NotNull{})`.
 * The return value is left opaque to the user and should at best be stored in
 * a type-deduced value (`auto`).
 */
template <auto AttributePtr, typename... Tags>
auto make_column(std::string name, Tags... tagattributes)
{
  using class_t =
      typename meta::AttributePtrDissector<decltype(AttributePtr)>::class_t;
  using attribute_t =
      typename meta::AttributePtrDissector<decltype(AttributePtr)>::attribute_t;
  constexpr auto tags = ColumnConstraints{tagattributes...};
  return Column<class_t, attribute_t, AttributePtr>{std::move(name), tags};
}
}

#endif /* !MYSQL_ORM_COLUMN_HPP_ */
