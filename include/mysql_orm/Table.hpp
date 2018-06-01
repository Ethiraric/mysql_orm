#ifndef MYSQL_ORM_TABLE_HPP
#define MYSQL_ORM_TABLE_HPP

#include <sstream>
#include <string>
#include <tuple>
#include <utility>

#include <mysql_orm/Column.hpp>
#include <mysql_orm/Utils.hpp>

namespace mysql_orm
{
template <typename... Columns>
class Table
{
public:
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

private:
  std::string table_name;
  std::tuple<Columns...> columns;
};

template <typename... Columns>
auto make_table(std::string name, Columns&&... cols)
{
  return Table<Columns...>{std::move(name), std::forward<Columns>(cols)...};
}
}

#endif /* !MYSQL_ORM_TABLE_HPP */
