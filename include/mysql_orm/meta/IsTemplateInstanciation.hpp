#ifndef MYSQL_ORM_META_ISTEMPLATEINSTANCIATION_HPP_
#define MYSQL_ORM_META_ISTEMPLATEINSTANCIATION_HPP_

#include <type_traits>

namespace mysql_orm
{
namespace meta
{
/** Metafunction returning whether the type is an instanciation of the template.
 *
 * For instance:
 *   - `IsTemplateInstanciation<std::vector, std::vector<int>>` == `true`
 *   - `IsTemplateInstanciation<std::map, std::optional<double>>` == `false`
 */
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
