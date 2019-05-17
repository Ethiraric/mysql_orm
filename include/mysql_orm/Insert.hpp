#ifndef MYSQL_ORM_INSERT_HPP_
#define MYSQL_ORM_INSERT_HPP_

#include <functional>
#include <sstream>

#include <mysql/mysql.h>

#include <mysql_orm/QueryType.hpp>
#include <mysql_orm/Statement.hpp>
#include <mysql_orm/meta/AttributePtrDissector.hpp>

namespace mysql_orm
{
template <typename Table, auto... Attrs>
class Insert
{
public:
  using model_type = typename Table::model_type;
  using table_type = Table;
  static inline constexpr auto query_type{QueryType::Insert};

  constexpr Insert(MYSQL& mysql,
                   Table const& t,
                   model_type const* to_insert) noexcept
    : mysql_handle{&mysql}, table{&t}, model_to_insert{to_insert}
  {
  }
  constexpr Insert(Insert const& b) noexcept = default;
  constexpr Insert(Insert&& b) noexcept = default;
  ~Insert() noexcept = default;

  Insert& operator=(Insert const& rhs) noexcept = default;
  Insert& operator=(Insert&& rhs) noexcept = default;

  auto operator()()
  {
    return this->build().execute();
  }

  constexpr auto buildquery() const
  {
    return this->buildqueryCS();
  }

  constexpr auto buildqueryCS() const
  {
    return this->table->template insertCS<Attrs...>();
  }

  constexpr Statement<Insert, model_type> build() const
  {
    auto stmt = Statement<Insert, model_type>{*this->mysql_handle, *this};
    if (this->model_to_insert)
      stmt.bindInsert(*this->model_to_insert);
    return stmt;
  }

  constexpr static size_t getNbInputSlots() noexcept
  {
    return sizeof...(Attrs);
  }

  constexpr static size_t getNbOutputSlots() noexcept
  {
    return 0;
  }

  template <std::size_t NBINDS>
  void bindInsert(model_type const& model, InputBindArray<NBINDS>& binds) const
      noexcept
  {
    auto i = std::size_t{0};
    (binds.bind(i++, model.*Attrs), ...);
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
  model_type const* model_to_insert;
};
}

#endif /* !MYSQL_ORM_INSERT_HPP_ */
