#ifndef MYSQL_ORM_INSERT_HPP_
#define MYSQL_ORM_INSERT_HPP_

#include <functional>
#include <sstream>

#include <mysql/mysql.h>

#include <mysql_orm/Statement.hpp>
#include <mysql_orm/StatementBinder.hpp>
#include <mysql_orm/meta/AttributePtrDissector.hpp>

namespace mysql_orm
{
template <typename Table, auto... Attrs>
class Insert
{
public:
  using model_type = typename Table::model_type;
  static inline constexpr auto is_select_query{false};

  Insert(MYSQL& mysql, Table const& t, model_type const* to_insert) noexcept
    : mysql_handle{&mysql}, table{t}, model_to_insert{to_insert}
  {
  }
  Insert(Insert const& b) noexcept = default;
  Insert(Insert&& b) noexcept = default;
  ~Insert() noexcept = default;

  Insert& operator=(Insert const& rhs) noexcept = default;
  Insert& operator=(Insert&& rhs) noexcept = default;

  auto operator()()
  {
    return this->build().execute();
  }

  std::string buildquery() const
  {
    return this->buildqueryss().str();
  }

  std::stringstream buildqueryss() const
  {
    return this->table.get().template insertss<Attrs...>();
  }

  Statement<Insert, model_type> build() const
  {
    auto stmt = Statement<Insert, model_type>{*this->mysql_handle, *this};
    if (this->model_to_insert)
      stmt.bindInsert(this->model_to_insert);
    return stmt;
  }

  constexpr size_t getNbInputSlots() const noexcept
  {
    return sizeof...(Attrs);
  }

  size_t getNbOutputSlots() const noexcept
  {
    return 0;
  }

  void bindInsert(model_type const& model, MYSQL_BIND* bindarray) const noexcept
  {
    (StatementInBinder<typename meta::AttributePtrDissector<decltype(
         Attrs)>::attribute_t>::bind(model.*Attrs, ++bindarray),
     ...);
  }

private:
  // May not be nullptr. Can't use std::reference_wrapper since MYSQL is
  // incomplete.
  MYSQL* mysql_handle;
  std::reference_wrapper<Table const> table;
  model_type const* model_to_insert;
};
}

#endif /* !MYSQL_ORM_INSERT_HPP_ */
