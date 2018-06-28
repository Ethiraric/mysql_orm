#ifndef MYSQL_ORM_UPDATE_HPP_
#define MYSQL_ORM_UPDATE_HPP_

#include <sstream>
#include <utility>

#include <mysql/mysql.h>

#include <mysql_orm/QueryType.hpp>
#include <mysql_orm/Set.hpp>
#include <mysql_orm/Statement.hpp>

namespace mysql_orm
{
template <typename Table>
class Update
{
public:
  using model_type = typename Table::model_type;
  static inline constexpr auto query_type{QueryType::Update};

  Update(MYSQL& mysql, Table const& t) noexcept : mysql_handle{&mysql}, table{t}
  {
  }
  Update(Update const& b) noexcept = default;
  Update(Update&& b) noexcept = default;
  ~Update() noexcept = default;

  Update& operator=(Update const& rhs) noexcept = default;
  Update& operator=(Update&& rhs) noexcept = default;

  std::stringstream buildqueryss() const
  {
    auto ss = std::stringstream{};
    ss << "UPDATE `" << this->table.get().getName() << "`";
    return ss;
  }

  template <typename Assignments>
  SetQuery<Update, Table, Assignments> operator()(Set<Assignments> set)
  {
    return SetQuery<Update, Table, Assignments>{
        *this->mysql_handle,
        *this,
        this->table.get(),
        std::move(set.assignments)};
  }

  auto operator()()
  {
    return this->build().execute();
  }

  size_t getNbInputSlots() const noexcept
  {
    return 0;
  }

  void bindInTo(InputBindArray&) const noexcept
  {
  }

private:
  // May not be nullptr. Can't use std::reference_wrapper since MYSQL is
  // incomplete.
  MYSQL* mysql_handle;
  std::reference_wrapper<Table const> table;
};
}

#endif /* !MYSQL_ORM_UPDATE_HPP_ */
