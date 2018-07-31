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

template <typename Model, typename Column, typename... Columns>
struct ColumnsMatchModel
  : std::conjunction<std::is_same<typename Column::model_type, Model>,
                     ColumnsMatchModel<Model, Columns...>>
{
};

template <typename Model, typename Column>
struct ColumnsMatchModel<Model, Column>
  : std::is_same<typename Column::model_type, Model>
{
};

template <typename Model, typename Column, typename... Columns>
inline constexpr auto ColumnsMatchModel_v =
    ColumnsMatchModel<Model, Column, Columns...>::value;

template <typename Column, typename... Columns>
struct ColumnsMatch : ColumnsMatchModel<typename Column::model_type, Columns...>
{
};

template <typename Column, typename... Columns>
inline constexpr auto ColumnsMatch_v = ColumnsMatch<Column, Columns...>::value;

template <typename Column, typename... Columns>
struct ColumnModel
{
  using type = typename Column::model_type;
};

template <typename... Columns>
using ColumnModel_t = typename ColumnModel<Columns...>::type;

namespace details
{
template <size_t idx, typename F, typename Acc,  typename... Ts>
constexpr auto tupleFoldl(F f, Acc const& acc, std::tuple<Ts...> const& t)
{
  if constexpr (idx == 0)
    return f(acc, std::get<0>(t));
  else
    return f(tupleFoldl<idx - 1>(f, acc, t), std::get<idx>(t));
}
}

template <typename F, typename Init, typename... Ts>
constexpr auto tupleFoldl(F f, Init const& init, std::tuple<Ts...> const& t)
{
  return details::tupleFoldl<sizeof...(Ts) - 1>(f, init, t);
}
}

#endif /* !MYSQL_ORM_UTILS_HPP_ */
