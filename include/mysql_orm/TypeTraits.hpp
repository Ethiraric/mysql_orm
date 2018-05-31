#ifndef MYSQL_ORM_TYPETRAITS_HPP_
#define MYSQL_ORM_TYPETRAITS_HPP_

namespace mysql_orm
{
template <typename AttributePtr>
struct AttributePtrDissector;

template <typename Class, typename Attribute>
struct AttributePtrDissector<Attribute Class::*>
{
  using class_t = Class;
  using attribute_t = Attribute;
};
}

#endif /* !MYSQL_ORM_TYPETRAITS_HPP_ */
