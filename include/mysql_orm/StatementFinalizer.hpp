#ifndef MYSQL_ORM_STATEMENTFINALIZER_HPP_
#define MYSQL_ORM_STATEMENTFINALIZER_HPP_

#include <cstring>
#include <string>
#include <type_traits>

#include <mysql/mysql.h>

#include <mysql_orm/TypeTraits.hpp>
#include <mysql_orm/meta/IsOptional.hpp>
#include <mysql_orm/meta/LiftOptional.hpp>

namespace mysql_orm
{
namespace details
{
template <typename Model, auto Attr>
struct SingleStatementFinalizer
{
  using attribute_t =
      typename AttributePtrDissector<decltype(Attr)>::attribute_t;
  static inline constexpr auto is_optional = meta::IsOptional_v<attribute_t>;
  using column_data_t = std::conditional_t<is_optional,
                                           meta::LiftOptional_t<attribute_t>,
                                           attribute_t>;

  static void finalize(Model& model,
                       MYSQL_BIND const& /*mysql_bind*/,
                       unsigned long& length,
                       my_bool& is_null,
                       my_bool& /*errorrray*/)
  {
    auto& attr = [&]() -> auto&
    {
      auto& field = model.*Attr;
      if constexpr (is_optional)
        return *field;
      else
        return field;
    }();
    static_assert(std::is_same_v<column_data_t, std::string> ||
                      std::is_same_v<column_data_t, char*> ||
                      std::is_integral_v<column_data_t>,
                  "Unknown type");

    if constexpr (is_optional)
    {
      if (is_null)
      {
        (model.*Attr).reset();
        return;
      }
    }

    if constexpr (std::is_same_v<column_data_t, std::string>)
    {
      if (is_null)
        attr.clear();
      else
        attr.resize(length);
    }
    else if constexpr (std::is_same_v<column_data_t, char*>)
    {
      auto* newtab = new char[length + 1];
      std::memcpy(newtab, attr, length);
      newtab[length] = '\0';
      delete[] attr;
      attr = newtab;
    }
  }
};
}

template <typename Model, auto Attr, auto... Attrs>
struct StatementFinalizer
{
  static void finalize(Model& model, MYSQL_BIND* bindarray,
                       unsigned long* lengtharray,
                       my_bool* is_null_array,
                       my_bool* errorarray)
  {
    details::SingleStatementFinalizer<Model, Attr>::finalize(
        model, bindarray[0], lengtharray[0], is_null_array[0], errorarray[0]);
    StatementFinalizer<Model, Attrs...>::finalize(model,
                                                  bindarray + 1,
                                                  lengtharray + 1,
                                                  is_null_array + 1,
                                                  errorarray + 1);
  }
};

template <typename Model, auto Attr>
struct StatementFinalizer<Model, Attr>
{
  static void finalize(Model& model,
                       MYSQL_BIND* bindarray,
                       unsigned long* lengtharray,
                       my_bool* is_null_array,
                       my_bool* errorarray)
  {
    details::SingleStatementFinalizer<Model, Attr>::finalize(
        model, bindarray[0], lengtharray[0], is_null_array[0], errorarray[0]);
  }
};
}

#endif /* !MYSQL_ORM_STATEMENTFINALIZER_HPP_ */
