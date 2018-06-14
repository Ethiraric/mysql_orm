#ifndef MYSQL_ORM_META_ISOPTIONAL_HPP_
#define MYSQL_ORM_META_ISOPTIONAL_HPP_

#include <optional>

#include <mysql_orm/meta/IsTemplateInstanciation.hpp>

namespace mysql_orm
{
namespace meta
{
template <typename T>
struct IsOptional : IsTemplateInstanciation<std::optional, T>
{
};

template <typename T>
inline constexpr auto IsOptional_v = IsOptional<T>::value;
}
}

#endif /* !MYSQL_ORM_META_ISOPTIONAL_HPP_ */
