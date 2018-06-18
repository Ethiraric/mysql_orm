#ifndef MYSQL_ORM_META_ATTRIBUTEPTRDISSECTOR_HPP_
#define MYSQL_ORM_META_ATTRIBUTEPTRDISSECTOR_HPP_

namespace mysql_orm
{
namespace meta
{
template <typename AttributePtr>
struct AttributePtrDissector;

template <typename Class, typename Attribute>
struct AttributePtrDissector<Attribute Class::*>
{
  using class_t = Class;
  using attribute_t = Attribute;
};

template <typename AttributePtr>
struct AttributeGetter
{
  using type = typename AttributePtrDissector<AttributePtr>::attribute_t;
};

template <typename AttributePtr>
using AttributeGetter_t = typename AttributeGetter<AttributePtr>::type;
}
}

#endif /* !MYSQL_ORM_META_ATTRIBUTEPTRDISSECTOR_HPP_ */
