#ifndef MYSQL_ORM_META_FINDMAPPED_HPP_
#define MYSQL_ORM_META_FINDMAPPED_HPP_

#include <type_traits>

#include <mysql_orm/meta/Map.hpp>
#include <mysql_orm/meta/TypeValEquals.hpp>

namespace mysql_orm
{
namespace meta
{
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
