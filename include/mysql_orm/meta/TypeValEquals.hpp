#ifndef MYSQL_ORM_META_TYPEVAREQUALS_HPP_
#define MYSQL_ORM_META_TYPEVAREQUALS_HPP_

#include <type_traits>

namespace mysql_orm
{
namespace meta
{
template <auto T, auto U>
struct TypeValEquals : std::false_type
{
};

template <auto T>
struct TypeValEquals<T, T> : std::true_type
{
};

template <auto T, auto U>
inline constexpr auto TypeValEquals_v = TypeValEquals<T, U>::value;
}
}

#endif /* !MYSQL_ORM_META_TYPEVAREQUALS_HPP_ */
