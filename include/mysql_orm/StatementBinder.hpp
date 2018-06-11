#ifndef MYSQL_ORM_STATEMENTBINDER_HPP_
#define MYSQL_ORM_STATEMENTBINDER_HPP_

#include <stdexcept>
#include <string>
#include <type_traits>

#include <mysql/mysql.h>

#include <mysql_orm/TypeTraits.hpp>

namespace mysql_orm
{
namespace details
{
template <typename Model, auto Attr>
struct SingleStatementBinder
{
  using attribute_t =
      typename AttributePtrDissector<decltype(Attr)>::attribute_t;

  static void bind(Model& model, MYSQL_BIND& mysql_bind)
  {
    auto& attr = model.*Attr;
    static_assert(std::is_same_v<attribute_t, std::string> ||
                      std::is_same_v<attribute_t, char*> ||
                      std::is_integral_v<attribute_t>,
                  "Unknown type");
    if constexpr (std::is_same_v<attribute_t, std::string>)
    {
      // XXX(ethiraric): Find a way to correctly allocate it.
      attr.resize(65536);
      mysql_bind.buffer_type = MYSQL_TYPE_STRING;
      mysql_bind.buffer = &attr[0];
      mysql_bind.buffer_length = 65536;
    }
    else if constexpr (std::is_same_v<attribute_t, char*>)
    {
      // XXX(ethiraric): Find a way to correctly allocate it.
      attr = new char[65536];
      mysql_bind.buffer_type = MYSQL_TYPE_STRING;
      mysql_bind.buffer = attr;
      mysql_bind.buffer_length = 65536;
    }
    else if constexpr (std::is_integral_v<attribute_t>)
    {
      static_assert(!std::is_integral_v<attribute_t> ||
                        (std::is_integral_v<attribute_t> &&
                         (sizeof(attr) == 1 || sizeof(attr) == 2 ||
                          sizeof(attr) == 4 || sizeof(attr == 8))),
                    "Unknown integer type");
      mysql_bind.is_unsigned = std::is_unsigned_v<attribute_t>;
      mysql_bind.buffer = &attr;
      mysql_bind.buffer_length = sizeof(attr);
      if constexpr (sizeof(attr) == 1)
        mysql_bind.buffer_type = MYSQL_TYPE_TINY;
      else if constexpr (sizeof(attr) == 2)
        mysql_bind.buffer_type = MYSQL_TYPE_SHORT;
      else if constexpr (sizeof(attr) == 4)
        mysql_bind.buffer_type = MYSQL_TYPE_LONG;
      else if constexpr (sizeof(attr) == 8)
        mysql_bind.buffer_type = MYSQL_TYPE_LONGLONG;
    }
  }
};
}

template <typename Model, auto Attr, auto... Attrs>
struct StatementBinder
{
  static void bind(Model& model, MYSQL_BIND* bindarray)
  {
    details::SingleStatementBinder<Model, Attr>::bind(model, bindarray[0]);
    StatementBinder<Model, Attrs...>::bind(model, bindarray + 1);
  }
};

template <typename Model, auto Attr>
struct StatementBinder<Model, Attr>
{
  static void bind(Model& model, MYSQL_BIND* bindarray)
  {
    details::SingleStatementBinder<Model, Attr>::bind(model, bindarray[0]);
  }
};
}

#endif /* !MYSQL_ORM_STATEMENTBINDER_HPP_ */
