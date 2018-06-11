#ifndef MYSQL_ORM_META_ALLSAME_HPP_
#define MYSQL_ORM_META_ALLSAME_HPP_

#include <type_traits>

namespace mysql_orm
{
namespace meta
{
template <typename T, typename U, typename... Ts>
struct AllSame
  : std::conditional_t<std::is_same_v<T, U>, AllSame<T, Ts...>, std::false_type>
{
};

template <typename T, typename U>
struct AllSame<T, U>
  : std::conditional_t<std::is_same_v<T, U>, std::true_type, std::false_type>
{
};

template <typename... Ts>
inline constexpr auto AllSame_v = AllSame<Ts...>::value;
}
}

#endif /* !MYSQL_ORM_META_ALLSAME_HPP_ */
