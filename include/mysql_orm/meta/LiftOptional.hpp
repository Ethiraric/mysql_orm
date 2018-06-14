#ifndef MYSQL_ORM_META_LIFTOPTIONAL_HPP_
#define MYSQL_ORM_META_LIFTOPTIONAL_HPP_

#include <optional>

namespace mysql_orm
{
namespace meta
{
template <typename T>
struct LiftOptional
{
  using type = std::nullopt_t;
};

template <typename T>
struct LiftOptional<std::optional<T>>
{
  using type = T;
};

template <typename T>
using LiftOptional_t = typename LiftOptional<T>::type;
}
}

#endif /* !MYSQL_ORM_META_LIFTOPTIONAL_HPP_ */
