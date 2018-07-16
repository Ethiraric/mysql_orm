#ifndef MYSQL_ORM_META_ALLSAME_HPP_
#define MYSQL_ORM_META_ALLSAME_HPP_

#include <type_traits>

namespace mysql_orm
{
namespace meta
{
/** Metafunction returning true if all types are the same.
 */
template <typename T, typename... Ts>
struct AllSame;

template <typename T>
struct AllSame<T> : std::true_type
{
};

template <typename T, typename U, typename... Ts>
struct AllSame<T, U, Ts...>
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
