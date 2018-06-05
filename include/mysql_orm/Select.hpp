#ifndef MYSQL_ORM_SELECT_HPP_
#define MYSQL_ORM_SELECT_HPP_

#include <sstream>
#include <utility>

#include <mysql_orm/Where.hpp>

namespace mysql_orm
{
template <typename Table, auto... Attrs>
class Select
{
public:
  explicit Select(Table const& t) noexcept : table{t}
  {
  }
  Select(Select const& b) noexcept = default;
  Select(Select&& b) noexcept = default;
  ~Select() noexcept = default;

  Select& operator=(Select const& rhs) noexcept = default;
  Select& operator=(Select&& rhs) noexcept = default;

  template <typename Condition>
  WhereQuery<Select, Table, Condition> operator()(Where<Condition> where)
  {
    return WhereQuery{*this, this->table.get(), std::move(where.condition)};
  }

  std::string build() const
  {
    return this->buildss().str();
  }

  std::stringstream buildss() const
  {
    return this->table.get().template selectss<Attrs...>();
  }

private:
  std::reference_wrapper<Table const> table;
};
}

#endif /* !MYSQL_ORM_SELECT_HPP_ */
