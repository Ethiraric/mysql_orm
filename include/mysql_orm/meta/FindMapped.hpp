#ifndef MYSQL_ORM_META_FINDMAPPED_HPP_
#define MYSQL_ORM_META_FINDMAPPED_HPP_

#include <type_traits>

#include <mysql_orm/meta/Map.hpp>
#include <mysql_orm/meta/TypeValEquals.hpp>

namespace mysql_orm
{
namespace meta
{
/** Metafunction returning the type whose mapped type is the Needle.
 *
 * Takes a list of types T. Applies a mapping metafunction to each of these
 * types, whose return type is U. Searches the Needle amongst all Us, and
 * returns the type T associated.
 * Returns void if not found.
 */
template <template <typename> typename Function,
          typename Needle,
          typename T,
          typename... List>
struct FindMapped
  : std::conditional<std::is_same_v<Needle, Map_t<Function, T>>,
                     T,
                     typename FindMapped<Function, Needle, List...>::type>
{
};

template <template <typename> typename Function, typename Needle, typename T>
struct FindMapped<Function, Needle, T>
  : std::conditional<std::is_same_v<Needle, Map_t<Function, T>>, T, void>
{
};

/** Metafunction returning the type whose mapped value is the Needle.
 *
 * Takes a list of types T. Applies a mapping metafunction to each of these
 * types, whose return value is V. Searches the Needle amongst all Vs, and
 * returns the type T associated.
 * Returns void if not found.
 */
template <template <typename> typename Function,
          auto Needle,
          typename T,
          typename... List>
struct FindMappedValue
  : std::conditional<TypeValEquals<Needle, MapValue_v<Function, T>>::value,
                     T,
                     typename FindMappedValue<Function, Needle, List...>::type>
{
};

template <template <typename> typename Function, auto Needle, typename T>
struct FindMappedValue<Function, Needle, T>
  : std::conditional<TypeValEquals<Needle, MapValue_v<Function, T>>::value,
                     T,
                     void>
{
};
}
}

#endif /* !MYSQL_ORM_META_FINDMAPPED_HPP_ */
