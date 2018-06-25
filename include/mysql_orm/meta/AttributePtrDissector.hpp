#ifndef MYSQL_ORM_META_ATTRIBUTEPTRDISSECTOR_HPP_
#define MYSQL_ORM_META_ATTRIBUTEPTRDISSECTOR_HPP_

namespace mysql_orm
{
namespace meta
{
/** Metafunction returning the type of the class and field from an attribute.
 *
 * Takes as argument the type of a pointer to an attribute.
 * Contains two member types:
 *   - `class_t`: Class to which the attribute belongs.
 *   - `attribute_t`: Type of the field.
 */
template <typename AttributePtr>
struct AttributePtrDissector;

template <typename Class, typename Attribute>
struct AttributePtrDissector<Attribute Class::*>
{
  using class_t = Class;
  using attribute_t = Attribute;
};

/** Metafunction returning the type of the field from an attribute
 */
template <typename AttributePtr>
struct AttributeGetter
{
  using type = typename AttributePtrDissector<AttributePtr>::attribute_t;
};

template <typename AttributePtr>
using AttributeGetter_t = typename AttributeGetter<AttributePtr>::type;

/** Metafunction returning the type of the model from an attribute
 */
template <typename AttributePtr>
struct AttributeModelGetter
{
  using type = typename AttributePtrDissector<AttributePtr>::class_t;
};

template <typename AttributePtr>
using AttributeModelGetter_t =
    typename AttributeModelGetter<AttributePtr>::type;
}
}

#endif /* !MYSQL_ORM_META_ATTRIBUTEPTRDISSECTOR_HPP_ */
