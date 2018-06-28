#ifndef MYSQL_ORM_BINDARRAY_HPP_
#define MYSQL_ORM_BINDARRAY_HPP_

#include <cstring>
#include <type_traits>
#include <vector>

#include <mysql/mysql.h>

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
}

/** Managed array of input `MYSQL_BIND`s.
 *
 * Has utility methods to bind values.
 */
class InputBindArray
{
public:
  explicit InputBindArray(std::size_t nbinds) noexcept
    : binds(nbinds)
  {
    std::memset(&this->binds[0], 0, sizeof(MYSQL_BIND) * nbinds);
  }

  InputBindArray(InputBindArray const& b) = default;
  InputBindArray(InputBindArray&& b) noexcept = default;
  ~InputBindArray() noexcept = default;

  InputBindArray& operator=(InputBindArray const& rhs) = default;
  InputBindArray& operator=(InputBindArray&& rhs) noexcept = default;

  template <typename T>
  void bind(std::size_t idx, T const& value)
  {
    constexpr auto is_optional = meta::IsOptional_v<T>;
    using column_data_t =
        std::conditional_t<is_optional, meta::LiftOptional_t<T>, T>;

    auto& mysql_bind = this->binds[idx];
    static_assert(std::is_same_v<column_data_t, std::string> ||
                      std::is_same_v<column_data_t, char*> ||
                      std::is_same_v<column_data_t, char const*> ||
                      std::is_integral_v<column_data_t> ||
                      std::is_same_v<column_data_t, MYSQL_TIME>,
                  "Unknown type");
    if constexpr (std::is_same_v<column_data_t, std::string>)
    {
      mysql_bind.buffer_type = MYSQL_TYPE_STRING;
      mysql_bind.buffer = const_cast<char*>(value.data());
      mysql_bind.buffer_length = value.size();
    }
    else if constexpr (std::is_same_v<column_data_t, char*> ||
                       std::is_same_v<column_data_t, char const*>)
    {
      mysql_bind.buffer_type = MYSQL_TYPE_STRING;
      mysql_bind.buffer = const_cast<char*>(value);
      mysql_bind.buffer_length = strlen(value);
    }
    else if constexpr (std::is_integral_v<column_data_t>)
    {
      mysql_bind.is_unsigned = std::is_unsigned_v<column_data_t>;
      mysql_bind.buffer_type =
          details::getMySQLIntegralFieldType<column_data_t>();
      mysql_bind.buffer = const_cast<column_data_t*>(&value);
      mysql_bind.buffer_length = sizeof(value);
    }
  }

  bool empty() const noexcept
  {
    return this->binds.empty();
  }

  MYSQL_BIND const* data() const noexcept
  {
    return this->binds.data();
  }

private:
  std::vector<MYSQL_BIND> binds;
};

/** Managed array of input `MYSQL_BIND`s.
 *
 * Has utility methods to bind values.
 *
 * `std::string`s are `resize`d and `char*` are `new`d.
 * The `finalize` method resizes to correct sizes once the query has been
 * executed (using the lengths).
 */
class OutputBindArray
{
public:
  explicit OutputBindArray(std::size_t nbinds) noexcept
    : binds(nbinds), lengths(nbinds), is_null(nbinds), error(nbinds)
  {
    std::memset(&this->binds[0], 0, sizeof(MYSQL_BIND) * nbinds);
    for (auto i = std::size_t{0}; i < nbinds; ++i)
    {
      this->binds[i].length = &this->lengths[i];
      this->binds[i].is_null = &this->is_null[i];
      this->binds[i].error = &this->error[i];
    }
  }

  OutputBindArray(OutputBindArray const& b) = default;
  OutputBindArray(OutputBindArray&& b) noexcept = default;
  ~OutputBindArray() noexcept = default;

  OutputBindArray& operator=(OutputBindArray const& rhs) = default;
  OutputBindArray& operator=(OutputBindArray&& rhs) noexcept = default;

  template <typename T>
  void bind(std::size_t idx, T& value)
  {
    using attribute_t = T;
    constexpr auto is_optional = meta::IsOptional_v<attribute_t>;
    using column_data_t = std::conditional_t<is_optional,
                                             meta::LiftOptional_t<attribute_t>,
                                             attribute_t>;
    auto& attr = [&]() -> auto&
    {
      auto& field = value;
      if constexpr (is_optional)
      {
        if (!field)
          field.emplace();
        return *field;
      }
      else
        return field;
    }();
    auto& mysql_bind = this->binds[idx];
    static_assert(std::is_same_v<column_data_t, std::string> ||
                      std::is_same_v<column_data_t, char*> ||
                      std::is_integral_v<column_data_t> ||
                      std::is_same_v<column_data_t, MYSQL_TIME>,
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
      mysql_bind.buffer_type =
          details::getMySQLIntegralFieldType<column_data_t>();
      mysql_bind.buffer = &attr;
      mysql_bind.buffer_length = sizeof(attr);
    }
  }

  template <typename T>
  void finalize(std::size_t idx, T& value)
  {
    using attribute_t = T;
    constexpr auto is_optional = meta::IsOptional_v<attribute_t>;
    using column_data_t = std::conditional_t<is_optional,
                                             meta::LiftOptional_t<attribute_t>,
                                             attribute_t>;
    auto& attr = [&]() -> auto&
    {
      auto& field = value;
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
      if (this->isNull(idx))
      {
        value.reset();
        return;
      }
    }

    if constexpr (std::is_same_v<column_data_t, std::string>)
    {
      if (this->isNull(idx))
        attr.clear();
      else
        attr.resize(this->length(idx));
    }
    else if constexpr (std::is_same_v<column_data_t, char*>)
    {
      auto const length = this->length(idx);
      auto* newtab = new char[length + 1];
      std::memcpy(newtab, attr, length);
      newtab[length] = '\0';
      delete[] attr;
      attr = newtab;
    }
    else
      (void)(idx);
  }

  bool empty() const noexcept
  {
    return this->binds.empty();
  }

  MYSQL_BIND const* data() const noexcept
  {
    return this->binds.data();
  }

  unsigned long length(std::size_t idx) const noexcept
  {
    return this->lengths[idx];
  }

  bool isNull(std::size_t idx) const noexcept
  {
    return this->is_null[idx];
  }

  bool hasErrored(std::size_t idx) const noexcept
  {
    return this->error[idx];
  }

private:
  std::vector<MYSQL_BIND> binds;
  std::vector<unsigned long> lengths;
  std::vector<my_bool> is_null;
  std::vector<my_bool> error;
};
}

#endif /* !MYSQL_ORM_BINDARRAY_HPP_ */
