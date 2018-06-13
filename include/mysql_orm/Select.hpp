#ifndef MYSQL_ORM_SELECT_HPP_
#define MYSQL_ORM_SELECT_HPP_

#include <sstream>
#include <utility>

#include <mysql/mysql.h>

#include <mysql_orm/Statement.hpp>
#include <mysql_orm/StatementBinder.hpp>
#include <mysql_orm/StatementFinalizer.hpp>
#include <mysql_orm/Where.hpp>

namespace mysql_orm
{
template <typename Table, auto... Attrs>
class Select
{
public:
  using model_type = typename Table::model_type;

  Select(MYSQL& mysql, Table const& t) noexcept : mysql_handle{&mysql}, table{t}
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
    return WhereQuery{*this->mysql_handle,
                      *this,
                      this->table.get(),
                      std::move(where.condition)};
  }

  std::string buildquery() const
  {
    return this->buildqueryss().str();
  }

  std::stringstream buildqueryss() const
  {
    return this->table.get().template selectss<Attrs...>();
  }

  Statement<Select, model_type> build() const
  {
    return Statement<Select, model_type>{*this->mysql_handle, *this};
  }

  size_t getNbOutputSlots() const noexcept
  {
    return sizeof...(Attrs) > 0 ? sizeof...(Attrs) : this->table.get().getNbColumns();
  }

  void bindOutTo(model_type& model, std::vector<MYSQL_BIND>& out_binds) const
  {
    if constexpr (sizeof...(Attrs) > 0)
      StatementBinder<model_type, Attrs...>::bind(model, &out_binds[0]);
    else
      Table::bindAll(model, out_binds);
  }

  void finalizeBindings(model_type& model,
                        std::vector<MYSQL_BIND>& binds,
                        std::vector<unsigned long>& lengths,
                        std::vector<my_bool>& are_null,
                        std::vector<my_bool>& errors)
  {
    if constexpr (sizeof...(Attrs) > 0)
    StatementFinalizer<model_type, Attrs...>::finalize(
        model, &binds[0], &lengths[0], &are_null[0], &errors[0]);
    else
      Table::finalizeAll(model, binds, lengths, are_null, errors);
  }

private:
  // May not be nullptr. Can't use std::reference_wrapper since MYSQL is
  // incomplete.
  MYSQL* mysql_handle;
  std::reference_wrapper<Table const> table;
};
}

#endif /* !MYSQL_ORM_SELECT_HPP_ */
