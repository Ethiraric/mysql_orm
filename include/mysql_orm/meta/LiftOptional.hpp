#ifndef MYSQL_ORM_META_LIFTOPTIONAL_HPP_
#define MYSQL_ORM_META_LIFTOPTIONAL_HPP_

#include <optional>

namespace mysql_orm
{
namespace meta
{
/** Metafunction returning the type inside a std::optional.
 *
 * If the given type is not an optional, returns std::nullopt_t.
 */
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
