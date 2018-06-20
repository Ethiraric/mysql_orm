#ifndef MYSQL_ORM_TABLE_HPP
#define MYSQL_ORM_TABLE_HPP

#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <mysql/mysql.h>

#include <mysql_orm/Column.hpp>
#include <mysql_orm/Select.hpp>
#include <mysql_orm/StatementBinder.hpp>
#include <mysql_orm/StatementFinalizer.hpp>
#include <mysql_orm/Utils.hpp>
#include <mysql_orm/meta/ColumnAttributeGetter.hpp>
#include <mysql_orm/meta/FindMapped.hpp>

namespace mysql_orm
{
template <typename Table, auto Attr, auto... Attrs>
struct ColumnNamesJoiner
{
  static std::string join(Table const& t)
  {
    return '`' + t.template getColumn<Attr>().getName() + "`, " +
           ColumnNamesJoiner<Table, Attrs...>::join(t);
  }
};

template <typename Table, auto Attr>
struct ColumnNamesJoiner<Table, Attr>
{
  static std::string join(Table const& t)
  {
    return '`' + t.template getColumn<Attr>().getName() + '`';
  }
};

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

  auto select(MYSQL& mysql) const
  {
    return Select<Table,
                  meta::MapValue_v<meta::ColumnAttributeGetter, Columns>...>(
        mysql, *this);
  }

  template <auto... Attrs>
  auto select(MYSQL& mysql) const
  {
    return Select<Table, Attrs...>(mysql, *this);
  }

  template <auto... Attrs>
  std::stringstream selectss() const
  {
    return SelectQueryBuilder<void, Attrs...>::select(*this);
  }

  template <auto Attr>
  auto const& getColumn() const noexcept
  {
    using Column_t = typename meta::
        FindMappedValue<meta::ColumnAttributeGetter, Attr, Columns...>::type;
    static_assert(!std::is_same_v<Column_t, void>, "Failed to find attribute");
    return std::get<Column_t>(this->columns);
  }

  static void bindAll(model_type& model, std::vector<MYSQL_BIND>& out_binds)
  {
    StatementOutBinder<model_type,
                       meta::MapValue_v<meta::ColumnAttributeGetter,
                                        Columns>...>::bind(model,
                                                           &out_binds[0]);
  }

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
  template <typename Dummy = void, auto... Attrs>
  struct SelectQueryBuilder
  {
    static std::stringstream select(Table const& t)
    {
      auto ss = std::stringstream{};

      ss << "SELECT " << ColumnNamesJoiner<Table, Attrs...>::join(t) << ' ';
      ss << "FROM `" << t.table_name << '`';
      return ss;
    }
  };

  template <typename Dummy>
  struct SelectQueryBuilder<Dummy>
  {
    static std::stringstream select(Table const& t)
    {
      auto ss = std::stringstream{};
      auto i = std::size_t{0};

      ss << "SELECT ";
      for_each_tuple(t.columns, [&](auto const& col) {
        ss << '`' << col.getName() << '`';
        if (++i < std::tuple_size<decltype(t.columns)>())
          ss << ',' << ' ';
      });
      ss << " FROM `" << t.table_name << '`';
      return ss;
    }
  };

  std::string const table_name;
  std::tuple<Columns...> columns;
};

template <typename... Columns>
auto make_table(std::string name, Columns&&... cols)
{
  return Table<Columns...>{std::move(name), std::forward<Columns>(cols)...};
}
}

#endif /* !MYSQL_ORM_TABLE_HPP */
