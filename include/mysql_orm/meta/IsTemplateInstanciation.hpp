#ifndef MYSQL_ORM_META_ISTEMPLATEINSTANCIATION_HPP_
#define MYSQL_ORM_META_ISTEMPLATEINSTANCIATION_HPP_

#include <type_traits>

namespace mysql_orm
{
namespace meta
{
template <template <typename...> typename Tpl, typename T>
struct IsTemplateInstanciation : std::false_type
{
};

template <template <typename...> typename Tpl, typename... Args>
struct IsTemplateInstanciation<Tpl, Tpl<Args...>> : std::true_type
{
};

template <template <typename...> typename Tpl, typename T>
using IsTemplateInstanciation_v =
    typename IsTemplateInstanciation<Tpl, T>::value;
}
}

#endif /* !MYSQL_ORM_META_ISTEMPLATEINSTANCIATION_HPP_ */
