#ifndef MYSQL_ORM_COLUMN_HPP_
#define MYSQL_ORM_COLUMN_HPP_

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

#include <CompileString/CompileString.hpp>
#include <CompileString/ToString.hpp>

#include <mysql_orm/ColumnConstraints.hpp>
#include <mysql_orm/meta/AttributePtrDissector.hpp>
#include <mysql_orm/meta/IsOptional.hpp>
#include <mysql_orm/meta/LiftOptional.hpp>

namespace mysql_orm
{
using id_t = uint32_t;

/** Returns a CompileString with the SQL type of the given field.
 *
 * The function errors when the type is unsupported.
 */
template <typename Field>
constexpr auto getFieldSQLType()
{
  if constexpr (std::is_same_v<Field, std::string> ||
                std::is_same_v<Field, char*> ||
                std::is_same_v<Field, char const*>)
    return compile_string::CompileString{"TEXT"};
  else if constexpr (std::is_integral_v<Field>)
  {
    if constexpr (std::is_same_v<Field, bool>)
      return compile_string::CompileString{"BOOLEAN"};
    else if constexpr (std::is_same_v<Field, int8_t>)
      return compile_string::CompileString{"TINYINT"};
    else if constexpr (std::is_same_v<Field, uint8_t>)
      return compile_string::CompileString{"TINYINT UNSIGNED"};
    else if constexpr (std::is_same_v<Field, int16_t>)
      return compile_string::CompileString{"SMALLINT"};
    else if constexpr (std::is_same_v<Field, uint16_t>)
      return compile_string::CompileString{"SMALLINT UNSIGNED"};
    else if constexpr (std::is_same_v<Field, int32_t>)
      return compile_string::CompileString{"INTEGER"};
    else if constexpr (std::is_same_v<Field, uint32_t>)
      return compile_string::CompileString{"INTEGER UNSIGNED"};
    else if constexpr (std::is_same_v<Field, int64_t>)
      return compile_string::CompileString{"BIGINT"};
    else if constexpr (std::is_same_v<Field, uint64_t>)
      return compile_string::CompileString{"BIGINT UNSIGNED"};
  }
  else if constexpr (std::is_same_v<Field, std::tm>)
    return compile_string::CompileString{"DATETIME"};
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
template <typename Model,
          typename Field,
          Field Model::*attr,
          typename ConstraintsPack,
          std::size_t NAME_SIZE,
          std::size_t VARCHAR_SIZE = 0>
class Column
{
public:
  using model_type = Model;
  using field_type = Field;
  using lifted_field_type = meta::LiftOptional_t<Field>;
  static inline constexpr auto attribute = attr;
  static inline constexpr auto is_optional = meta::IsOptional_v<Field>;
  static inline constexpr auto varchar_size = VARCHAR_SIZE;

  constexpr Column(char const (&name)[NAME_SIZE]) noexcept
    : column_name{name}
  {
    if constexpr (varchar_size > 0 &&
                  !(std::is_same_v<lifted_field_type, std::string> ||
                    std::is_same_v<lifted_field_type, char*> ||
                    std::is_same_v<lifted_field_type, char const*>))
      throw std::runtime_error("VARCHAR can only be used for text types");
    // if (this->tags.nullable == Tristate::Undefined)
    //   this->tags.nullable = is_optional ? Tristate::On : Tristate::Off;
  }
  constexpr Column(Column const& b) = default;
  constexpr Column(Column&& b) noexcept = default;
  ~Column() noexcept = default;

  constexpr Column& operator=(Column const& rhs) = default;
  constexpr Column& operator=(Column&& rhs) noexcept = default;

  /** Returns the part of the create statement associated with the column.
   */
  constexpr auto getSchema() const
  {
    auto tagstr =
        columnConstraintsFromPack(ConstraintsPack{}).toString();
    auto schema = '`' + this->getName() + '`' + ' ';
    auto schema2 = [&]() {
      if constexpr (varchar_size > 0)
        return schema + "VARCHAR(" + compile_string::toString<varchar_size>() +
               ')';
      else
        return schema + getFieldSQLType<lifted_field_type>();
    }();
    if constexpr (!tagstr.empty())
      return schema2 + ' ' + tagstr;
    else
      return schema2;
  }

  constexpr auto getName() const noexcept
  {
    return this->column_name;
  }

private:
  template <std::size_t N>
  using CompileString = compile_string::CompileString<N>;

  CompileString<NAME_SIZE - 1> column_name;
};

/** Helper functions to create a column.
 *
 * Used as `make_column<&Model::Field>("column_name", Autoincrement{},
 * NotNull{})`.
 * The return value is left opaque to the user and should at best be stored in
 * a type-deduced value (`auto`).
 *
 * `make_varchar` is used to not store strings as `TEXT` types, but rather as
 * `VARCHAR`s.
 */
template <std::size_t varchar_size,
          auto AttributePtr,
          std::size_t name_size,
          typename... Constraints>
constexpr auto make_varchar(char const (&name)[name_size], Constraints...)
{
  using class_t =
      typename meta::AttributePtrDissector<decltype(AttributePtr)>::class_t;
  using attribute_t =
      typename meta::AttributePtrDissector<decltype(AttributePtr)>::attribute_t;
  return Column<class_t,
                attribute_t,
                AttributePtr,
                meta::Pack<Constraints...>,
                name_size,
                varchar_size>{name};
}

template <auto AttributePtr, std::size_t name_size, typename... Constraints>
constexpr auto make_column(char const (&name)[name_size],
                           Constraints... constraints)
{
  return make_varchar<0, AttributePtr>(name, constraints...);
}
}

#endif /* !MYSQL_ORM_COLUMN_HPP_ */
