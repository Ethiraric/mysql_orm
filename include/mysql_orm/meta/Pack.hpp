#ifndef MYSQL_ORM_META_PACK_HPP_
#define MYSQL_ORM_META_PACK_HPP_

#include <type_traits>

#include <mysql_orm/meta/TypeValEquals.hpp>

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

/** Returns true if the pack contains the given type.
 */
template <typename Needle, typename ZePack>
struct PackContains;

template <typename Needle>
struct PackContains<Needle, Pack<>> : std::false_type
{
};

template <typename Needle, typename T, typename... Ts>
struct PackContains<Needle, Pack<T, Ts...>>
  : std::conditional_t<std::is_same_v<Needle, T>,
                       std::true_type,
                       PackContains<Needle, Pack<Ts...>>>
{
};

template <typename Needle, typename ZePack>
inline constexpr auto PackContains_v = PackContains<Needle, ZePack>::value;

/** Empty struct. Used to manipulate template parameter packs.
 */
template <auto... Vs>
struct ValuePack
{
  static inline constexpr auto size = sizeof...(Vs);
};

/** Merges values from both packs into another third pack.
 */
template <typename P1, typename P2>
struct MergeValuePacks;

template <auto... Vs, auto... Ws>
struct MergeValuePacks<ValuePack<Vs...>, ValuePack<Ws...>>
{
  using type = ValuePack<Vs..., Ws...>;
};

template <typename P1, typename P2>
using MergeValuePacks_t = typename MergeValuePacks<P1, P2>::type;

/** Append a type at the end of a pack.
 */
template <typename ZeValuePack, auto V>
struct AppendValuePack;

template <auto... Vs, auto V>
struct AppendValuePack<ValuePack<Vs...>, V>
{
  using type = ValuePack<Vs..., V>;
};

template <typename ZeValuePack, auto V>
using AppendValuePack_t = typename AppendValuePack<ZeValuePack, V>::type;

/** Prepend a type at the beginning of a pack.
 */
template <typename ZeValuePack, auto V>
struct PrependValuePack;

template <auto... Vs, auto V>
struct PrependValuePack<ValuePack<Vs...>, V>
{
  using type = ValuePack<V, Vs...>;
};

template <typename ZeValuePack, auto V>
using PrependValuePack_t = typename PrependValuePack<ZeValuePack, V>::type;

/** Filter the types inside a pack.
 */
template <template <auto> typename Predicate, typename ZeValuePack>
struct FilterValuePack;

template <template <auto> typename Filter, auto V, auto... Vs>
struct FilterValuePack<Filter, ValuePack<V, Vs...>>
  : std::conditional<
        Filter<V>::value,
        typename PrependValuePack<
            typename FilterValuePack<Filter, ValuePack<Vs...>>::type,
            V>::type,
        typename FilterValuePack<Filter, ValuePack<Vs...>>::type>
{
};

template <template <auto> typename Filter>
struct FilterValuePack<Filter, ValuePack<>>
{
  using type = ValuePack<>;
};

template <template <auto> typename Filter, typename ZeValuePack>
using FilterValuePack_t = typename FilterValuePack<Filter, ZeValuePack>::type;

/** Returns true if the pack contains the given value.
 */
template <auto Needle, typename ZeValuePack>
struct ValuePackContains;

template <auto Needle>
struct ValuePackContains<Needle, ValuePack<>> : std::false_type
{
};

template <auto Needle, auto V, auto... Vs>
struct ValuePackContains<Needle, ValuePack<V, Vs...>>
  : std::conditional_t<TypeValEquals_v<Needle, V>,
                       std::true_type,
                       ValuePackContains<Needle, ValuePack<Vs...>>>
{
};

template <auto Needle, typename ZePack>
inline constexpr auto ValuePackContains_v =
    ValuePackContains<Needle, ZePack>::value;
}
}

#endif /* !MYSQL_ORM_META_PACK_HPP_ */
