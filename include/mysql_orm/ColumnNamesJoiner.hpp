#ifndef MYSQL_ORM_TABLE_HPP_
#define MYSQL_ORM_TABLE_HPP_

#include <string>

namespace mysql_orm
{
namespace details
{
/** Joins the names of the attributes' columns.
 */
template <typename Table, auto Attr, auto... Attrs>
struct ColumnNamesJoiner
{
  static auto join(Table const& t)
  {
    return "`" + t.template getColumn<Attr>().getName() + "`, " +
           ColumnNamesJoiner<Table, Attrs...>::join(t);
  }
};

template <typename Table, auto Attr>
struct ColumnNamesJoiner<Table, Attr>
{
  static auto join(Table const& t)
  {
    return "`" + t.template getColumn<Attr>().getName() + "`";
  }
};
}
}

#endif /* !MYSQL_ORM_TABLE_HPP_ */
