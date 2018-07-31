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

  constexpr GetAll(MYSQL& mysql, Table const& t) noexcept
    : mysql_handle{&mysql}, table{&t}
  {
  }
  constexpr GetAll(GetAll const& b) noexcept = default;
  constexpr GetAll(GetAll&& b) noexcept = default;
  ~GetAll() noexcept = default;

  constexpr GetAll& operator=(GetAll const& rhs) noexcept = default;
  constexpr GetAll& operator=(GetAll&& rhs) noexcept = default;

  template <typename Condition>
  constexpr WhereQuery<GetAll, Table, Condition> operator()(
      Where<Condition> where)
  {
    return WhereQuery<GetAll, Table, Condition>{
        *this->mysql_handle, *this, *this->table, std::move(where.condition)};
  }

  template <typename Limit>
  constexpr LimitQuery<GetAll, Table, Limit> operator()(Limit limit)
  {
    return LimitQuery<GetAll, Table, Limit>{
        *this->mysql_handle, *this, *this->table, std::move(limit)};
  }

  auto operator()()
  {
    return this->build().execute();
  }

  constexpr auto buildquery() const noexcept
  {
    return this->buildqueryCS();
  }

  constexpr auto buildqueryCS() const noexcept
  {
    return this->table->template selectCS<Attrs...>();
  }

  constexpr Statement<GetAll, model_type> build() const
  {
    return Statement<GetAll, model_type>{*this->mysql_handle, *this};
  }

  constexpr static size_t getNbInputSlots() noexcept
  {
    return 0;
  }

  constexpr static size_t getNbOutputSlots() noexcept
  {
    return sizeof...(Attrs);
  }

  template <std::size_t NBINDS>
  void bindOutTo(model_type& model, OutputBindArray<NBINDS>& binds) const
  {
    auto i = std::size_t{0};
    (binds.template bind<std::remove_reference_t<decltype(
         table->template getColumn<Attrs>())>::varchar_size>(i++, model.*Attrs),
     ...);
  }

  template <std::size_t NBINDS>
  constexpr void bindInTo(InputBindArray<NBINDS>&) const noexcept
  {
  }

  template <std::size_t NBINDS>
  constexpr void rebindStdTmReferences(InputBindArray<NBINDS>&) const noexcept
  {
  }

  template <std::size_t NBINDS>
  constexpr void finalizeBindings(model_type& model,
                                  OutputBindArray<NBINDS>& binds)
  {
    auto i = std::size_t{0};
    (binds.finalize(i++, model.*Attrs), ...);
  }

private:
  // May not be nullptr. Can't use std::reference_wrapper since MYSQL is
  // incomplete.
  MYSQL* mysql_handle;
  Table const* table;
};
}

#endif /* !MYSQL_ORM_GETALL_HPP_ */
