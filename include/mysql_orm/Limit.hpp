#ifndef MYSQL_ORM_LIMIT_HPP_
#define MYSQL_ORM_LIMIT_HPP_

#include <cstddef>
#include <functional>
#include <sstream>

#include <mysql/mysql.h>

#include <mysql_orm/QueryContinuation.hpp>

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
class LimitQueryImpl
{
public:
  using model_type = typename Query::model_type;

  LimitQueryImpl(MYSQL& mysql, Query q, Table const& t, Limit&& l) noexcept
    : mysql_handle{&mysql}, query{std::move(q)}, table{t}, limit{std::move(l)}
  {
  }
  LimitQueryImpl(LimitQueryImpl const& b) noexcept = default;
  LimitQueryImpl(LimitQueryImpl&& b) noexcept = default;
  ~LimitQueryImpl() noexcept = default;

  LimitQueryImpl& operator=(LimitQueryImpl const& rhs) noexcept = default;
  LimitQueryImpl& operator=(LimitQueryImpl&& rhs) noexcept = default;

  std::stringstream buildqueryss() const
  {
    auto ss = this->query.buildqueryss();
    ss << " LIMIT " << this->limit.value;
    return ss;
  }

protected:
  // May not be nullptr. Can't use std::reference_wrapper since MYSQL is
  // incomplete.
  MYSQL* mysql_handle;
  Query query;
  std::reference_wrapper<Table const> table;

private:
  Limit limit;
};

template <typename Query, typename Table, typename Limit>
using LimitQuery =
    QueryContinuation<Query, Table, LimitQueryImpl<Query, Table, Limit>>;
}

#endif /* !MYSQL_ORM_LIMIT_HPP_ */
