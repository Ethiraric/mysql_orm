#ifndef MYSQL_ORM_TABLE_HPP
#define MYSQL_ORM_TABLE_HPP

#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <mysql/mysql.h>

#include <mysql_orm/Column.hpp>
#include <mysql_orm/ColumnNamesJoiner.hpp>
#include <mysql_orm/Insert.hpp>
#include <mysql_orm/GetAll.hpp>
#include <mysql_orm/Utils.hpp>
#include <mysql_orm/meta/ColumnAttributeGetter.hpp>
#include <mysql_orm/meta/FindMapped.hpp>
#include <mysql_orm/meta/Pack.hpp>
#include <mysql_orm/meta/RemoveOccurences.hpp>

namespace mysql_orm
{
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
template <typename... Columns>
class Table
{
public:
  using model_type = ColumnModel_t<Columns...>;

  explicit Table(std::string name, Columns&&... cols)
    : table_name{std::move(name)}, columns{std::forward_as_tuple(cols...)}
  {
    static_assert(ColumnsMatch_v<Columns...>,
                  "Columns do not refer to the same model type");
  }
  Table(Table const& b) = default;
  Table(Table&& b) noexcept = default;
  ~Table() noexcept = default;

  Table& operator=(Table const& rhs) = default;
  Table& operator=(Table&& rhs) noexcept = default;

  /** Returns a SQL statement to create the table.
   */
  std::string getSchema() const
  {
    auto ss = std::stringstream{};
    auto i = std::size_t{0};

    ss << "CREATE TABLE `" << this->table_name << '`' << ' ' << '(';
    for_each_tuple(this->columns, [&](auto const& col) {
      ss << '\n' << ' ' << ' ' << col.getSchema();
      if (++i < std::tuple_size<decltype(this->columns)>())
        ss << ',';
    });
    ss << '\n' << ')';
    return ss.str();
  }

  std::string const& getName() const noexcept
  {
    return this->table_name;
  }

  /** Returns a query to select all fields from the table.
   */
  auto getAll(MYSQL& mysql) const
  {
    return GetAll<Table,
                  meta::MapValue_v<meta::ColumnAttributeGetter, Columns>...>(
        mysql, *this);
  }

  /** Returns a query to select some fields from the table.
   */
  template <auto... Attrs>
  auto getAll(MYSQL& mysql) const
  {
    this->checkAttributes<Attrs...>();
    return GetAll<Table, Attrs...>(mysql, *this);
  }

  template <auto... Attrs>
  auto insertAllBut(MYSQL& mysql, model_type const* model = nullptr) const
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
  auto insert(MYSQL& mysql, model_type const* model = nullptr) const
  {
    return Insert<Table,
                  meta::MapValue_v<meta::ColumnAttributeGetter, Columns>...>(
        mysql, *this, model);
  }

  template <auto... Vs>
  auto insert(meta::ValuePack<Vs...>,
              MYSQL& mysql,
              model_type const* model) const
  {
    this->checkAttributes<Vs...>();
    return this->insert<Vs...>(mysql, model);
  }

  /** Returns a query to insert some fields into the table.
   */
  template <auto... Attrs>
  auto insert(MYSQL& mysql, model_type const* model = nullptr) const
  {
    this->checkAttributes<Attrs...>();
    return Insert<Table, Attrs...>(mysql, *this, model);
  }

  /** Returns a stringstream with the select query for specified fields.
   */
  template <auto... Attrs>
  std::stringstream selectss() const
  {
    this->checkAttributes<Attrs...>();
    return SelectQueryBuilder<void, Attrs...>::select(*this);
  }

  /** Returns a stringstream with the insert query for specified fields.
   */
  template <auto... Attrs>
  std::stringstream insertss() const
  {
    this->checkAttributes<Attrs...>();
    return InsertQueryBuilder<void, Attrs...>::insert(*this);
  }

  /** Returns the column associated to the specified attribute.
   */
  template <auto Attr>
  auto const& getColumn() const noexcept
  {
    using Column_t = typename meta::
        FindMappedValue<meta::ColumnAttributeGetter, Attr, Columns...>::type;
    static_assert(!std::is_same_v<Column_t, void>, "Failed to find attribute");
    return std::get<Column_t>(this->columns);
  }

  size_t getNbColumns() const noexcept
  {
    return sizeof...(Columns);
  }

private:
  template <auto... Attrs>
  void checkAttributes() const noexcept
  {
    using AttrsPack = meta::ValuePack<
        meta::MapValue_v<meta::ColumnAttributeGetter, Columns>...>;
    static_assert((meta::ValuePackContains_v<Attrs, AttrsPack> && ...),
                  "Failed to find attribute");
  }

  /** Returns a stringstream with a SELECT query for the given attributes.
   */
  template <typename Dummy = void, auto... Attrs>
  struct SelectQueryBuilder
  {
    static std::stringstream select(Table const& t)
    {
      auto ss = std::stringstream{};

      ss << "SELECT " << details::ColumnNamesJoiner<Table, Attrs...>::join(t)
         << ' ';
      ss << "FROM `" << t.table_name << '`';
      return ss;
    }
  };

  /** Returns a stringstream with a INSERT query for the given attributes.
   */
  template <typename Dummy = void, auto... Attrs>
  struct InsertQueryBuilder
  {
    static std::stringstream insert(Table const& t)
    {
      auto ss = std::stringstream{};

      ss << "INSERT INTO `" << t.table_name << '`' << ' ' << '('
         << details::ColumnNamesJoiner<Table, Attrs...>::join(t)
         << ") VALUES (";
      for (auto i = std::size_t{0}; i < sizeof...(Attrs); ++i)
      {
        if (i)
          ss << ',' << ' ';
        ss << '?';
      }
      ss << ')';
      return ss;
    }
  };

  std::string const table_name;
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
template <typename... Columns>
auto make_table(std::string name, Columns&&... cols)
{
  return Table<Columns...>{std::move(name), std::forward<Columns>(cols)...};
}
}

#endif /* !MYSQL_ORM_TABLE_HPP */
