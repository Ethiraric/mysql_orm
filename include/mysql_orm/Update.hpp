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

  constexpr Update(MYSQL& mysql, Table const& t) noexcept : mysql_handle{&mysql}, table{&t}
  {
  }
  constexpr Update(Update const& b) noexcept = default;
  constexpr Update(Update&& b) noexcept = default;
  ~Update() noexcept = default;

  constexpr Update& operator=(Update const& rhs) noexcept = default;
  constexpr Update& operator=(Update&& rhs) noexcept = default;

  constexpr auto buildqueryCS() const
  {
    return "UPDATE `" + this->table->getName() + "`";
  }

  template <typename Assignments>
  constexpr SetQuery<Update, Table, Assignments> operator()(
      Set<Assignments> set)
  {
    return SetQuery<Update, Table, Assignments>{*this->mysql_handle,
                                                *this,
                                                *this->table,
                                                std::move(set.assignments)};
  }

  auto operator()()
  {
    return this->build().execute();
  }

  constexpr static size_t getNbOutputSlots() noexcept
  {
    return 0;
  }

  constexpr static size_t getNbInputSlots() noexcept
  {
    return 0;
  }

  template <std::size_t NBINDS>
  constexpr void bindInTo(InputBindArray<NBINDS>&) const noexcept
  {
  }

  template <std::size_t NBINDS>
  constexpr void rebindStdTmReferences(InputBindArray<NBINDS>&) const noexcept
  {
  }

private:
  // May not be nullptr. Can't use std::reference_wrapper since MYSQL is
  // incomplete.
  MYSQL* mysql_handle;
  Table const* table;
};
}

#endif /* !MYSQL_ORM_UPDATE_HPP_ */
