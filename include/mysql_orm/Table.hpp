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
#include <mysql_orm/Select.hpp>
#include <mysql_orm/StatementBinder.hpp>
#include <mysql_orm/StatementFinalizer.hpp>
#include <mysql_orm/Utils.hpp>
#include <mysql_orm/meta/ColumnAttributeGetter.hpp>
#include <mysql_orm/meta/FindMapped.hpp>

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
  auto select(MYSQL& mysql) const
  {
    return Select<Table,
                  meta::MapValue_v<meta::ColumnAttributeGetter, Columns>...>(
        mysql, *this);
  }

  /** Returns a query to select some fields from the table.
   */
  template <auto... Attrs>
  auto select(MYSQL& mysql) const
  {
    return Select<Table, Attrs...>(mysql, *this);
  }

  /** Returns a query to insert all fields into the table.
   */
  auto insert(MYSQL& mysql, model_type const* model = nullptr) const
  {
    return Insert<Table,
                  meta::MapValue_v<meta::ColumnAttributeGetter, Columns>...>(
        mysql, *this, model);
  }

  /** Returns a query to insert some fields into the table.
   */
  template <auto... Attrs>
  auto insert(MYSQL& mysql, model_type const* model = nullptr) const
  {
    return Insert<Table, Attrs...>(mysql, *this, model);
  }

  /** Returns a stringstream with the select query for specified fields.
   */
  template <auto... Attrs>
  std::stringstream selectss() const
  {
    return SelectQueryBuilder<void, Attrs...>::select(*this);
  }

  /** Returns a stringstream with the insert query for specified fields.
   */
  template <auto... Attrs>
  std::stringstream insertss() const
  {
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

  /** Binds all the MySQL output fields.
   */
  static void bindAll(model_type& model, std::vector<MYSQL_BIND>& out_binds)
  {
    StatementOutBinder<model_type,
                       meta::MapValue_v<meta::ColumnAttributeGetter,
                                        Columns>...>::bind(model,
                                                           &out_binds[0]);
  }

  /** Performs last-minute operations on fields before copying.
   *
   * This mostly boils down to correctly assigning lengths to TEXT fields
   * (std::string::resize and reallocating C-style arrays).
   */
  static void finalizeAll(model_type& model,
                          std::vector<MYSQL_BIND>& binds,
                          std::vector<unsigned long>& lengths,
                          std::vector<my_bool>& are_null,
                          std::vector<my_bool>& errors)
  {
    StatementFinalizer<model_type,
                       meta::MapValue_v<meta::ColumnAttributeGetter,
                                        Columns>...>::finalize(model,
                                                               &binds[0],
                                                               &lengths[0],
                                                               &are_null[0],
                                                               &errors[0]);
  }

  size_t getNbColumns() const noexcept
  {
    return sizeof...(Columns);
  }

private:
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
