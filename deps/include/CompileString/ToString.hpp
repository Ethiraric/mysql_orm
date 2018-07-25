#ifndef COMPILESTRING_TOSTRING_HPP_
#define COMPILESTRING_TOSTRING_HPP_

#include <cstddef>

#include <type_traits>

#include <CompileString/CompileString.hpp>

namespace compile_string
{
namespace details
{
template <auto i>
constexpr auto unsignedIntegerToString()
{
  if constexpr(i >= 10)
    return unsignedIntegerToString<i / 10>() +
           ('0' + static_cast<char>(i % 10));
  else
    return CompileString{""} + ('0' + static_cast<char>(i));
}
}

template <auto i>
constexpr auto toString()
{
  static_assert(std::is_integral_v<decltype(i)>);
  if constexpr (i < 0)
  {
    auto num = details::unsignedIntegerToString<-(i + 1)>();
    ++num.back();
    return CompileString{"-"} + num;
  }
  else
    return details::unsignedIntegerToString<i>();
}
}

#endif /* !COMPILESTRING_TOSTRING_HPP_ */
