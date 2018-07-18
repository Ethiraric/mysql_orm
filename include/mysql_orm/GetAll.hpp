#ifndef MYSQL_ORM_GETALL_HPP_
#define MYSQL_ORM_GETALL_HPP_

#include <sstream>
#include <utility>

#include <mysql/mysql.h>

#include <mysql_orm/Limit.hpp>
#include <mysql_orm/QueryType.hpp>
#include <mysql_orm/Statement.hpp>
#include <mysql_orm/Where.hpp>

namespace mysql_orm
{
/** A GetAll query.
 *
 * The class selects all attributes that are given in template arguments.
 * All attributes must refer to the model of the table for the program to
 * compile.
 *
 * This query returns objects as opposed to raw columns.
 *
 * `buildquery` returns the SQL query as a std::string.
 * `build` returns a `Statement`, which can later be `execute()`d.
 *
 * The `operator()` can be used to continue the query (Where, Limit).
 */
template <typename Table, auto... Attrs>
class GetAll
{
public:
  using model_type = typename Table::model_type;
  static inline constexpr auto query_type{QueryType::GetAll};

  GetAll(MYSQL& mysql, Table const& t) noexcept : mysql_handle{&mysql}, table{t}
  {
  }
  GetAll(GetAll const& b) noexcept = default;
  GetAll(GetAll&& b) noexcept = default;
  ~GetAll() noexcept = default;

  GetAll& operator=(GetAll const& rhs) noexcept = default;
  GetAll& operator=(GetAll&& rhs) noexcept = default;

  template <typename Condition>
  WhereQuery<GetAll, Table, Condition> operator()(Where<Condition> where)
  {
    return WhereQuery<GetAll, Table, Condition>{*this->mysql_handle,
                                                *this,
                                                this->table.get(),
                                                std::move(where.condition)};
  }

  template <typename Limit>
  LimitQuery<GetAll, Table, Limit> operator()(Limit limit)
  {
    return LimitQuery<GetAll, Table, Limit>{
        *this->mysql_handle, *this, this->table.get(), std::move(limit)};
  }

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
    return this->table.get().template selectss<Attrs...>();
  }

  Statement<GetAll, model_type> build() const
  {
    return Statement<GetAll, model_type>{*this->mysql_handle, *this};
  }

  size_t getNbInputSlots() const noexcept
  {
    return 0;
  }

  size_t getNbOutputSlots() const noexcept
  {
    return sizeof...(Attrs);
  }

  void bindOutTo(model_type& model, OutputBindArray& binds) const
  {
    auto i = std::size_t{0};
    (binds.bind<std::remove_reference_t<decltype(
         table.get().template getColumn<Attrs>())>::varchar_size>(i++,
                                                                  model.*Attrs),
     ...);
  }

  void bindInTo(InputBindArray&) const noexcept
  {
  }

  void rebindStdTmReferences(InputBindArray&) const noexcept
  {
  }

  void finalizeBindings(model_type& model, OutputBindArray& binds)
  {
    auto i = std::size_t{0};
    (binds.finalize(i++, model.*Attrs), ...);
  }

private:
  // May not be nullptr. Can't use std::reference_wrapper since MYSQL is
  // incomplete.
  MYSQL* mysql_handle;
  std::reference_wrapper<Table const> table;
};
}

#endif /* !MYSQL_ORM_GETALL_HPP_ */
