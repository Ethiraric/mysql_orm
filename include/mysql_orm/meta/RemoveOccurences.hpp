#ifndef MYSQL_ORM_META_REMOVEOCCURENCES_HPP_
#define MYSQL_ORM_META_REMOVEOCCURENCES_HPP_

#include <type_traits>

#include <mysql_orm/meta/Pack.hpp>
#include <mysql_orm/meta/TypeValEquals.hpp>

namespace mysql_orm
{
namespace meta
{
template <typename HayPack, typename NeedlePack>
struct RemoveOccurences;

template <typename... Hays>
struct RemoveOccurences<Pack<Hays...>, Pack<>>
{
  using type = Pack<Hays...>;
};

template <typename Hay, typename Needle>
struct RemoveOccurences<Pack<Hay>, Pack<Needle>>
  : std::conditional<std::is_same_v<Hay, Needle>, Pack<>, Pack<Hay>>
{
};

template <typename Hay, typename... Hays, typename Needle>
struct RemoveOccurences<Pack<Hay, Hays...>, Pack<Needle>>
  : std::conditional<
        std::is_same_v<Hay, Needle>,
        typename RemoveOccurences<Pack<Hays...>, Pack<Needle>>::type,
        PrependPack_t<
            typename RemoveOccurences<Pack<Hays...>, Pack<Needle>>::type,
            Hay>>
{
};

template <typename... Hays, typename Needle, typename... Needles>
struct RemoveOccurences<Pack<Hays...>, Pack<Needle, Needles...>>
  : RemoveOccurences<
        typename RemoveOccurences<Pack<Hays...>, Pack<Needle>>::type,
        Pack<Needles...>>
{
};

template <typename HayPack, typename NeedlePack>
using RemoveOccurences_t = typename RemoveOccurences<HayPack, NeedlePack>::type;

template <typename HayPack, typename NeedlePack>
struct RemoveValueOccurences;

template <auto... Hays>
struct RemoveValueOccurences<ValuePack<Hays...>, ValuePack<>>
{
  using type = ValuePack<Hays...>;
};

template <auto Hay, auto Needle>
struct RemoveValueOccurences<ValuePack<Hay>, ValuePack<Needle>>
  : std::conditional<TypeValEquals_v<Hay, Needle>, ValuePack<>, ValuePack<Hay>>
{
};

template <auto Hay, auto... Hays, auto Needle>
struct RemoveValueOccurences<ValuePack<Hay, Hays...>, ValuePack<Needle>>
  : std::conditional<TypeValEquals_v<Hay, Needle>,
                     typename RemoveValueOccurences<ValuePack<Hays...>,
                                                    ValuePack<Needle>>::type,
                     PrependValuePack_t<typename RemoveValueOccurences<
                                            ValuePack<Hays...>,
                                            ValuePack<Needle>>::type,
                                        Hay>>
{
};

template <auto... Hays, auto Needle, auto... Needles>
struct RemoveValueOccurences<ValuePack<Hays...>, ValuePack<Needle, Needles...>>
  : RemoveValueOccurences<
        typename RemoveValueOccurences<ValuePack<Hays...>,
                                       ValuePack<Needle>>::type,
        ValuePack<Needles...>>
{
};

template <typename HayPack, typename NeedlePack>
using RemoveValueOccurences_t =
    typename RemoveValueOccurences<HayPack, NeedlePack>::type;
}
}

#endif /* !MYSQL_ORM_META_REMOVEOCCURENCES_HPP_ */
