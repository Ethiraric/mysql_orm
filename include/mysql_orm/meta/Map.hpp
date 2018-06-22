#ifndef MYSQL_ORM_META_MAP_HPP_
#define MYSQL_ORM_META_MAP_HPP_

namespace mysql_orm
{
namespace meta
{
/** Metafunction applying a function (returning a type) to a type.
 */
template <template <typename> typename Function, typename Type>
struct Map
{
  using type = typename Function<Type>::type;
};

template <template <typename> typename Function, typename Type>
using Map_t = typename Map<Function, Type>::type;

/** Metafunction applying a function (returning a value) to a type.
 */
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
