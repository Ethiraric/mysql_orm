#ifndef MYSQL_ORM_UTILS_HPP_
#define MYSQL_ORM_UTILS_HPP_

#include <tuple>
#include <utility>

namespace mysql_orm
{
template <std::size_t I = 0, typename F, typename... Tp>
std::enable_if_t<I == sizeof...(Tp)> for_each_tuple(std::tuple<Tp...> const&, F)
{
}

template <std::size_t I = 0, typename F, typename... Tp>
std::enable_if_t<I != sizeof...(Tp)> for_each_tuple(std::tuple<Tp...> const& t,
                                                    F f)
{
  f(std::get<I>(t));
  for_each_tuple<I + 1, F, Tp...>(t, f);
}
}

#endif /* !MYSQL_ORM_UTILS_HPP_ */
