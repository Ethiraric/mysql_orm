#ifndef MYSQL_ORM_TABLE_HPP
#define MYSQL_ORM_TABLE_HPP

#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <mysql/mysql.h>

#include <CompileString/CompileString.hpp>

#include <mysql_orm/Column.hpp>
#include <mysql_orm/ColumnNamesJoiner.hpp>
#include <mysql_orm/GetAll.hpp>
#include <mysql_orm/Insert.hpp>
#include <mysql_orm/Utils.hpp>
#include <mysql_orm/meta/ColumnAttributeGetter.hpp>
#include <mysql_orm/meta/FindMapped.hpp>
#include <mysql_orm/meta/Pack.hpp>
#include <mysql_orm/meta/RemoveOccurences.hpp>

namespace mysql_orm
{
namespace details
{
// Work around a clang bug with auto template method of templated class.
template <auto Attr, typename... Columns>
auto const& getColumn_clang(std::tuple<Columns...> const& cols) noexcept
{
  using Column_t = typename meta::
      FindMappedValue<meta::ColumnAttributeGetter, Attr, Columns...>::type;
  static_assert(!std::is_same_v<Column_t, void>, "Failed to find attribute");
  return std::get<Column_t>(cols);
}
}

/** A SQL Table.
 *
 * The table can be constructed with a name and columns (instanciations of the
 * `Column` template). Users should not manipulate this class directly.
 *
 * For the code to compile, all columns must refer to the same Model.
 *
 * The class defines one member type:
 *   - `model_type`: Alias to the Model the table refers to.
 */
template <std::size_t NAME_SIZE, typename... Columns>
class Table
{
public:
  using model_type = ColumnModel_t<Columns...>;

  constexpr explicit Table(char const (&name)[NAME_SIZE], Columns&&... cols)
    : table_name{name}, columns{std::forward_as_tuple(cols...)}
  {
    static_assert(ColumnsMatch_v<Columns...>,
                  "Columns do not refer to the same model type");
  }
  constexpr Table(Table const& b) = default;
  constexpr Table(Table&& b) = default;
  ~Table() noexcept = default;

  constexpr Table& operator=(Table const& rhs) = default;
  constexpr Table& operator=(Table&& rhs) = default;

  /** Returns a SQL statement to create the table.
   */
  constexpr auto getSchema() const
  {
    return "CREATE TABLE `" + this->table_name + "` (" +
           tupleFoldl(
               [](auto const& acc, auto const& column) {
                 if constexpr (std::is_same_v <
                                   compile_string::CompileString<0> const&,
                               decltype(acc)>)
                   return "\n  " + column.getSchema();
                 else
                   return acc + ",\n  " + column.getSchema();
               },
               CompileString<0>{""},
               this->columns) +
           "\n)";
  }

  constexpr auto getName() const noexcept
  {
    return this->table_name;
  }

  /** Returns a query to select all fields from the table.
   */
  constexpr auto getAll(MYSQL& mysql) const
  {
    return GetAll<Table,
                  meta::MapValue_v<meta::ColumnAttributeGetter, Columns>...>(
        mysql, *this);
  }

  /** Returns a query to select some fields from the table.
   */
  template <auto... Attrs>
  constexpr auto getAll(MYSQL& mysql) const
  {
    this->checkAttributes<Attrs...>();
    return GetAll<Table, Attrs...>(mysql, *this);
  }

  template <auto... Attrs>
  constexpr auto insertAllBut(MYSQL& mysql,
                              model_type const* model = nullptr) const
  {
    this->checkAttributes<Attrs...>();
    using PackType = meta::RemoveValueOccurences_t<
        meta::ValuePack<
            meta::MapValue_v<meta::ColumnAttributeGetter, Columns>...>,
        meta::ValuePack<Attrs...>>;
    return this->insert(PackType{}, mysql, model);
  }

  /** Returns a query to insert all fields into the table.
   */
  constexpr auto insert(MYSQL& mysql, model_type const* model = nullptr) const
  {
    return Insert<Table,
                  meta::MapValue_v<meta::ColumnAttributeGetter, Columns>...>(
        mysql, *this, model);
  }

  template <auto... Vs>
  constexpr auto insert(meta::ValuePack<Vs...>,
                        MYSQL& mysql,
                        model_type const* model) const
  {
    this->checkAttributes<Vs...>();
    return this->insert<Vs...>(mysql, model);
  }

  /** Returns a query to insert some fields into the table.
   */
  template <auto... Attrs>
  constexpr auto insert(MYSQL& mysql, model_type const* model = nullptr) const
  {
    this->checkAttributes<Attrs...>();
    return Insert<Table, Attrs...>(mysql, *this, model);
  }

  /** Returns a CompileString with the select query for specified fields.
   */
  template <auto... Attrs>
  constexpr auto selectCS() const
  {
    this->checkAttributes<Attrs...>();
    return SelectQueryBuilder<void, Attrs...>::select(*this);
  }

  /** Returns a CompileString with the insert query for specified fields.
   */
  template <auto... Attrs>
  constexpr auto insertCS() const
  {
    this->checkAttributes<Attrs...>();
    return InsertQueryBuilder<void, Attrs...>::insert(*this);
  }

  /** Returns the column associated to the specified attribute.
   */
  template <auto Attr>
  auto const& getColumn() const noexcept
  {
    return  details::getColumn_clang<Attr, Columns...>(this->columns);
  }

  constexpr size_t getNbColumns() const noexcept
  {
    return sizeof...(Columns);
  }

private:
  template <std::size_t N>
  using CompileString = compile_string::CompileString<N>;

  template <auto... Attrs>
  constexpr void checkAttributes() const noexcept
  {
    using AttrsPack = meta::ValuePack<
        meta::MapValue_v<meta::ColumnAttributeGetter, Columns>...>;
    static_assert((meta::ValuePackContains_v<Attrs, AttrsPack> && ...),
                  "Failed to find attribute");
  }

  /** Returns a CompileString with a SELECT query for the given attributes.
   */
  template <typename Dummy = void, auto... Attrs>
  struct SelectQueryBuilder
  {
    constexpr static auto select(Table const& t)
    {
      return "SELECT " + details::ColumnNamesJoiner<Table, Attrs...>::join(t) +
             " " + "FROM `" + t.table_name + "`";
    }
  };

  /** Returns a CompileString with a INSERT query for the given attributes.
   */
  template <typename Dummy = void, auto... Attrs>
  struct InsertQueryBuilder
  {
    constexpr static auto insert(Table const& t)
    {
      return "INSERT INTO `" + t.table_name + '`' + ' ' + '(' +
             details::ColumnNamesJoiner<Table, Attrs...>::join(t) +
             ") VALUES (" +
             applyN<sizeof...(Attrs)>(
                 [](auto const& acc) {
                   if constexpr (std::is_same_v<
                                     compile_string::CompileString<0> const&,
                                     decltype(acc)>)
                     return compile_string::CompileString{"?"};
                   else
                     return acc + ", ?";
                 },
                 CompileString<0>{""}) +
             ')';
    }
  };

  CompileString<NAME_SIZE - 1> table_name;
  std::tuple<Columns...> columns;
};

/** Helper function to create a table.
 *
 * Used the following way:
 * ```
 * make_table(make_column<&Model::Field1>("field1", PrimaryKey{}),
 *            make_column<&Model::Field2>("field2", NotNull{}),
 *            make_column<&Model::Field3>("field3")))
 * ```
 *
 * The return value is left opaque to the user and should at best be stored in
 * a type-deduced value (`auto`).
 */
template <typename... Columns, std::size_t name_size>
constexpr auto make_table(char const (&name)[name_size], Columns&&... cols)
{
  return Table<name_size, Columns...>{name, std::forward<Columns>(cols)...};
}
}

#endif /* !MYSQL_ORM_TABLE_HPP */
