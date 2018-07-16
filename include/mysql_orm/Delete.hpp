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

  Delete(MYSQL& mysql, Table const& t) noexcept : mysql_handle{&mysql}, table{t}
  {
  }
  Delete(Delete const& b) noexcept = default;
  Delete(Delete&& b) noexcept = default;
  ~Delete() noexcept = default;

  Delete& operator=(Delete const& rhs) noexcept = default;
  Delete& operator=(Delete&& rhs) noexcept = default;

  std::string buildquery() const
  {
    return this->buildqueryss().str();
  }

  std::stringstream buildqueryss() const
  {
    auto ss = std::stringstream{};
    ss << "DELETE FROM `" << this->table.get().getName() << "`";
    return ss;
  }

  Statement<Delete, model_type> build() const
  {
    return Statement<Delete, model_type>{*this->mysql_handle, *this};
  }

  template <typename Condition>
  WhereQuery<Delete, Table, Condition> operator()(Where<Condition> where)
  {
    return WhereQuery<Delete, Table, Condition>{*this->mysql_handle,
                                                *this,
                                                this->table.get(),
                                                std::move(where.condition)};
  }

  template <typename Limit>
  LimitQuery<Delete, Table, Limit> operator()(Limit limit)
  {
    return LimitQuery<Delete, Table, Limit>{
        *this->mysql_handle, *this, this->table.get(), std::move(limit)};
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

  void rebindStdTmReferences(InputBindArray&) const noexcept
  {
  }

private:
  // May not be nullptr. Can't use std::reference_wrapper since MYSQL is
  // incomplete.
  MYSQL* mysql_handle;
  std::reference_wrapper<Table const> table;
};
}

#endif /* !MYSQL_ORM_DELETE_HPP_ */
