#ifndef MYSQL_ORM_DELETE_HPP_
#define MYSQL_ORM_DELETE_HPP_

#include <functional>
#include <sstream>

#include <mysql/mysql.h>

#include <mysql_orm/Limit.hpp>
#include <mysql_orm/QueryType.hpp>
#include <mysql_orm/Where.hpp>

namespace mysql_orm
{
template <typename Table>
class Delete
{
public:
  using model_type = typename Table::model_type;
  static inline constexpr auto query_type{QueryType::Delete};

  constexpr Delete(MYSQL& mysql, Table const& t) noexcept : mysql_handle{&mysql}, table{&t}
  {
  }
  constexpr Delete(Delete const& b) noexcept = default;
  constexpr Delete(Delete&& b) noexcept = default;
  ~Delete() noexcept = default;

  constexpr Delete& operator=(Delete const& rhs) noexcept = default;
  constexpr Delete& operator=(Delete&& rhs) noexcept = default;

  constexpr auto buildquery() const noexcept
  {
    return this->buildqueryCS();
  }

  constexpr auto buildqueryCS() const noexcept
  {
    return "DELETE FROM `" + this->table->getName() + "`";
  }

  constexpr Statement<Delete, model_type> build() const
  {
    return Statement<Delete, model_type>{*this->mysql_handle, *this};
  }

  template <typename Condition>
  constexpr WhereQuery<Delete, Table, Condition> operator()(Where<Condition> where)
  {
    return WhereQuery<Delete, Table, Condition>{*this->mysql_handle,
                                                *this,
                                                *this->table,
                                                std::move(where.condition)};
  }

  template <typename Limit>
  constexpr LimitQuery<Delete, Table, Limit> operator()(Limit limit)
  {
    return LimitQuery<Delete, Table, Limit>{
        *this->mysql_handle, *this, *this->table, std::move(limit)};
  }

  auto operator()()
  {
    return this->build().execute();
  }

  static constexpr size_t getNbInputSlots() noexcept
  {
    return 0;
  }

  static constexpr size_t getNbOutputSlots() noexcept
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

#endif /* !MYSQL_ORM_DELETE_HPP_ */
