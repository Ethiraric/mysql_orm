#ifndef MYSQL_ORM_META_MAP_HPP_
#define MYSQL_ORM_META_MAP_HPP_

namespace mysql_orm
{
namespace meta
{
template <template <typename> typename Function, typename Type>
struct Map
{
  using type = typename Function<Type>::type;
};

template <template <typename> typename Function, typename Type>
using Map_t = typename Map<Function, Type>::type;

template <template <typename> typename Function, typename Type>
struct MapValue
{
  static inline constexpr auto value = Function<Type>::value;
};

template <template <typename> typename Function, typename Type>
inline constexpr auto MapValue_v = MapValue<Function, Type>::value;
}
}

#endif /* !MYSQL_ORM_META_MAP_HPP_ */
