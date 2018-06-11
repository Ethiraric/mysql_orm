#ifndef MYSQL_ORM_META_COLUMNATTRIBUTEGETTER_HPP_
#define MYSQL_ORM_META_COLUMNATTRIBUTEGETTER_HPP_

namespace mysql_orm
{
namespace meta
{
template <typename Column>
struct ColumnAttributeGetter
{
  static inline constexpr auto value = Column::attribute;
};
}
}

#endif /* !MYSQL_ORM_META_COLUMNATTRIBUTEGETTER_HPP_ */
