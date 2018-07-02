#ifndef MYSQL_ORM_META_PACK_HPP_
#define MYSQL_ORM_META_PACK_HPP_

#include <type_traits>

namespace mysql_orm
{
namespace meta
{
/** Empty struct. Used to manipulate template parameter packs.
 */
template <typename... Ts>
struct Pack
{
  static inline constexpr auto size = sizeof...(Ts);
};

/** Merges types from both packs into another third pack.
 */
template <typename P1, typename P2>
struct MergePacks;

template <typename... Ts, typename... Us>
struct MergePacks<Pack<Ts...>, Pack<Us...>>
{
  using type = Pack<Ts..., Us...>;
};

template <typename P1, typename P2>
using MergePacks_t = typename MergePacks<P1, P2>::type;

/** Append a type at the end of a pack.
 */
template <typename ZePack, typename T>
struct AppendPack;

template <typename... Ts, typename T>
struct AppendPack<Pack<Ts...>, T>
{
  using type = Pack<Ts..., T>;
};

template <typename ZePack, typename T>
using AppendPack_t = typename AppendPack<ZePack, T>::type;

/** Prepend a type at the beginning of a pack.
 */
template <typename ZePack, typename T>
struct PrependPack;

template <typename... Ts, typename T>
struct PrependPack<Pack<Ts...>, T>
{
  using type = Pack<T, Ts...>;
};

template <typename ZePack, typename T>
using PrependPack_t = typename PrependPack<ZePack, T>::type;

/** Filter the types inside a pack.
 */
template <template <typename> typename Predicate, typename ZePack>
struct FilterPack;

template <template <typename> typename Filter, typename T, typename... Ts>
struct FilterPack<Filter, Pack<T, Ts...>>
  : std::conditional<
        Filter<T>::value,
        typename PrependPack<typename FilterPack<Filter, Pack<Ts...>>::type,
                             T>::type,
        typename FilterPack<Filter, Pack<Ts...>>::type>
{
};

template <template <typename> typename Filter>
struct FilterPack<Filter, Pack<>>
{
  using type = Pack<>;
};

template <template <typename> typename Filter, typename ZePack>
using FilterPack_t = typename FilterPack<Filter, ZePack>::type;
}
}

#endif /* !MYSQL_ORM_META_PACK_HPP_ */
