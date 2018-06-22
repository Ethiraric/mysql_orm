#ifndef MYSQL_ORM_META_TABLEMODELGETTER_HPP_
#define MYSQL_ORM_META_TABLEMODELGETTER_HPP_

#include <type_traits>

namespace mysql_orm
{
namespace meta
{
/** Metafunction returning the model type from the table.
 */
template <typename Table>
struct TableModelGetter
{
  using type = typename std::remove_reference_t<Table>::model_type;
};

template <typename Table>
using TableModelGetter_t = typename TableModelGetter<Table>::type;
}
}

#endif /* !MYSQL_ORM_META_TABLEMODELGETTER_HPP_ */
