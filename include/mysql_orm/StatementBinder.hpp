#ifndef MYSQL_ORM_STATEMENTBINDER_HPP_
#define MYSQL_ORM_STATEMENTBINDER_HPP_

#include <stdexcept>
#include <string>
#include <type_traits>

#include <mysql/mysql.h>

#include <mysql_orm/meta/AttributePtrDissector.hpp>
#include <mysql_orm/meta/IsOptional.hpp>
#include <mysql_orm/meta/LiftOptional.hpp>

namespace mysql_orm
{
namespace details
{
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr enum_field_types getMySQLIntegralFieldType()
{
  if constexpr (sizeof(T) == 1)
    return MYSQL_TYPE_TINY;
  else if constexpr (sizeof(T) == 2)
    return MYSQL_TYPE_SHORT;
  else if constexpr (sizeof(T) == 4)
    return MYSQL_TYPE_LONG;
  else if constexpr (sizeof(T) == 8)
    return MYSQL_TYPE_LONGLONG;
}

template <typename Model, auto Attr>
struct SingleStatementOutBinder
{
  using attribute_t =
      typename meta::AttributePtrDissector<decltype(Attr)>::attribute_t;
  static inline constexpr auto is_optional = meta::IsOptional_v<attribute_t>;
  using column_data_t = std::conditional_t<is_optional,
                                           meta::LiftOptional_t<attribute_t>,
                                           attribute_t>;

  static void bind(Model& model, MYSQL_BIND& mysql_bind)
  {
    auto& attr = [&]() -> auto&
    {
      auto& field = model.*Attr;
      if constexpr (is_optional)
      {
        if (!field)
          field.emplace();
        return *field;
      }
      else
        return field;
    }
    ();
    static_assert(std::is_same_v<column_data_t, std::string> ||
                      std::is_same_v<column_data_t, char*> ||
                      std::is_integral_v<column_data_t>,
                  "Unknown type");
    if constexpr (std::is_same_v<column_data_t, std::string>)
    {
      // XXX(ethiraric): Find a way to correctly allocate it.
      attr.resize(65536);
      mysql_bind.buffer_type = MYSQL_TYPE_STRING;
      mysql_bind.buffer = &attr[0];
      mysql_bind.buffer_length = 65536;
    }
    else if constexpr (std::is_same_v<column_data_t, char*>)
    {
      // XXX(ethiraric): Find a way to correctly allocate it.
      attr = new char[65536];
      mysql_bind.buffer_type = MYSQL_TYPE_STRING;
      mysql_bind.buffer = attr;
      mysql_bind.buffer_length = 65536;
    }
    else if constexpr (std::is_integral_v<column_data_t>)
    {
      mysql_bind.is_unsigned = std::is_unsigned_v<column_data_t>;
      mysql_bind.buffer_type = getMySQLIntegralFieldType<column_data_t>();
      mysql_bind.buffer = &attr;
      mysql_bind.buffer_length = sizeof(attr);
    }
  }
};
}

template <typename Model, auto Attr, auto... Attrs>
struct StatementOutBinder
{
  static void bind(Model& model, MYSQL_BIND* bindarray)
  {
    details::SingleStatementOutBinder<Model, Attr>::bind(model, bindarray[0]);
    StatementOutBinder<Model, Attrs...>::bind(model, bindarray + 1);
  }
};

template <typename Model, auto Attr>
struct StatementOutBinder<Model, Attr>
{
  static void bind(Model& model, MYSQL_BIND* bindarray)
  {
    details::SingleStatementOutBinder<Model, Attr>::bind(model, bindarray[0]);
  }
};

template <typename T>
struct StatementInBinder
{
  static inline constexpr auto is_optional = meta::IsOptional_v<T>;
  using column_data_t =
      std::conditional_t<is_optional, meta::LiftOptional_t<T>, T>;
  static void bind(T const& value, MYSQL_BIND* pbind) noexcept
  {
    auto& bind = *pbind;
    static_assert(std::is_same_v<column_data_t, std::string> ||
                      std::is_same_v<column_data_t, char*> ||
                      std::is_same_v<column_data_t, char const*> ||
                      std::is_integral_v<column_data_t>,
                  "Unknown type");
    if constexpr (std::is_same_v<column_data_t, std::string>)
    {
      bind.buffer_type = MYSQL_TYPE_STRING;
      bind.buffer = const_cast<char*>(value.data());
      bind.buffer_length = value.size();
    }
    else if constexpr (std::is_same_v<column_data_t, char*> ||
                       std::is_same_v<column_data_t, char const*>)
    {
      bind.buffer_type = MYSQL_TYPE_STRING;
      bind.buffer = const_cast<char*>(value);
      bind.buffer_length = strlen(value);
    }
    else if constexpr (std::is_integral_v<column_data_t>)
    {
      bind.is_unsigned = std::is_unsigned_v<column_data_t>;
      bind.buffer_type = details::getMySQLIntegralFieldType<column_data_t>();
      bind.buffer = const_cast<column_data_t*>(&value);
      bind.buffer_length = sizeof(value);
    }
  }
};
}

#endif /* !MYSQL_ORM_STATEMENTBINDER_HPP_ */
