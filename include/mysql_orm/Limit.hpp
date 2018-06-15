#ifndef MYSQL_ORM_LIMIT_HPP_
#define MYSQL_ORM_LIMIT_HPP_

#include <cstddef>
#include <functional>
#include <sstream>

#include <mysql/mysql.h>

#include <mysql_orm/Statement.hpp>

namespace mysql_orm
{
template <size_t limit = 0>
struct Limit
{
  static inline constexpr size_t value{limit};
};

template <>
struct Limit<0>
{
  size_t const value{0};
};

template <typename Query, typename Table, typename Limit>
class LimitQuery
{
public:
  using model_type = typename Query::model_type;

  LimitQuery(MYSQL& mysql, Query q, Table const& t, Limit&& l) noexcept
    : mysql_handle{&mysql},
      query{std::move(q)},
      table{t},
      limit{std::move(l)}
  {
  }
  LimitQuery(LimitQuery const& b) noexcept = default;
  LimitQuery(LimitQuery&& b) noexcept = default;
  ~LimitQuery() noexcept = default;

  LimitQuery& operator=(LimitQuery const& rhs) noexcept = default;
  LimitQuery& operator=(LimitQuery&& rhs) noexcept = default;

  std::stringstream buildqueryss() const
  {
    auto ss = this->query.buildqueryss();
    ss << " LIMIT " << this->limit.value;
    return ss;
  }

  std::string buildquery() const
  {
    return this->buildqueryss().str();
  }

  Statement<LimitQuery, model_type> build() const
  {
    return Statement<LimitQuery, model_type>{*this->mysql_handle, *this};
  }

  size_t getNbOutputSlots() const noexcept
  {
    return this->query.getNbOutputSlots();
  }

  void bindOutTo(model_type& model, std::vector<MYSQL_BIND>& out_binds) const
  {
    this->query.bindOutTo(model, out_binds);
  }

  void finalizeBindings(model_type& model,
                        std::vector<MYSQL_BIND>& binds,
                        std::vector<unsigned long>& lengths,
                        std::vector<my_bool>& are_null,
                        std::vector<my_bool>& errors)
  {
    this->query.finalizeBindings(model, binds, lengths, are_null, errors);
  }

private:
  // May not be nullptr. Can't use std::reference_wrapper since MYSQL is
  // incomplete.
  MYSQL* mysql_handle;
  Query query;
  std::reference_wrapper<Table const> table;
  Limit limit;
};
}

#endif /* !MYSQL_ORM_LIMIT_HPP_ */
