#ifndef MYSQL_ORM_LIMIT_HPP_
#define MYSQL_ORM_LIMIT_HPP_

#include <cstddef>
#include <functional>
#include <sstream>

#include <CompileString/CompileString.hpp>
#include <CompileString/ToString.hpp>
#include <mysql/mysql.h>

#include <mysql_orm/QueryContinuation.hpp>

namespace mysql_orm
{
namespace details
{
constexpr unsigned int ulog10(std::size_t n)
{
  auto log = 0u;

  while (n > 10)
  {
    ++log;
    n /= 10;
  }

  return log;
}

constexpr auto toBigEnoughCS(std::size_t n) noexcept
{
  constexpr auto retlen = ulog10(std::numeric_limits<std::size_t>::max());
  auto ret = compile_string::CompileString<retlen>{
      compile_string::CompileString<0>{""}};
  for (auto i = std::size_t{0}; i < retlen - 1; ++i)
    ret[i] = ' ';
  ret[retlen - 1] = '0';
  for (auto i = retlen; i > 0 && n > 0; ++i, n /= 10)
    ret[i - 1] = '0' + n % 10;
  return ret;
}
}

/** Limit clause arguments.
 *
 * This class is used as an argument to a `Select`'s or `Where`'s `operator()`
 * The limit is embedded in the template argument.
 * If the limit is set to 0, a runtime value may be supplied upon
 * initialization.
 *
 * This class is not the actual query but a class that serves as a tag for the
 * other query classes.
 * The query class is `LimitQueryImpl`.
 */
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

/** A Limit query.
 *
 * The class continues a `Select` or `Where` query.
 * Takes a limit as argument, which must be a `Limit`.
 *
 * `buildquery` returns the SQL query as a std::string.
 * `build` returns a `Statement`, which can later be `execute()`d.
 */
template <typename Query, typename Table, typename TLimit>
class LimitQueryImpl
{
public:
  using model_type = typename Query::model_type;

  LimitQueryImpl(MYSQL& mysql, Query q, Table const& t, TLimit&& l) noexcept
    : mysql_handle{&mysql}, query{std::move(q)}, table{t}, limit{std::move(l)}
  {
  }
  LimitQueryImpl(LimitQueryImpl const& b) = default;
  LimitQueryImpl(LimitQueryImpl&& b) noexcept = default;
  ~LimitQueryImpl() noexcept = default;

  LimitQueryImpl& operator=(LimitQueryImpl const& rhs) = default;
  LimitQueryImpl& operator=(LimitQueryImpl&& rhs) noexcept = default;

  auto buildqueryCS() const
  {
    /// XXX(ethiraric): Use the real limit!!!
    if constexpr (!std::is_same_v<TLimit, Limit<0>>)
      return this->query.buildqueryCS() + " LIMIT " +
             compile_string::toString<TLimit::value>();
    else
      return this->query.buildqueryCS() + " LIMIT " +
             details::toBigEnoughCS(limit.value);
  }

protected:
  // May not be nullptr. Can't use std::reference_wrapper since MYSQL is
  // incomplete.
  MYSQL* mysql_handle;
  Query query;
  std::reference_wrapper<Table const> table;

private:
  TLimit limit;
};

template <typename Query, typename Table, typename Limit>
using LimitQuery =
    QueryContinuation<Query, Table, LimitQueryImpl<Query, Table, Limit>>;
}

#endif /* !MYSQL_ORM_LIMIT_HPP_ */
