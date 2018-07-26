#ifndef COMPILESTRING_COMPILESTRING_HPP_
#define COMPILESTRING_COMPILESTRING_HPP_

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>

namespace compile_string
{
inline constexpr auto npos = std::size_t(-1);

/** Compile time string.
 *
 * Provides operations on raw c-style strings.
 * This class is NOT a non-owning view (like std::string_view or
 * gsl::string_span are). It has its own buffer for storing the string.
 *
 * The past-the-end iterator is considered to point to the '\0'.
 * This means that [begin(), end()) iterates through all N characters.
 */
template <std::size_t N>
class CompileString
{
public:
  using value_type = char;
  using pointer = value_type*;
  using const_pointer = value_type const*;
  using size_type = std::size_t;
  using index_type = size_type;
  using container_type = std::array<char, N + 1>;
  using iterator = typename container_type::iterator;
  using const_iterator = typename container_type::const_iterator;
  using reverse_iterator = typename container_type::reverse_iterator;
  using const_reverse_iterator =
      typename container_type::const_reverse_iterator;

  static inline constexpr auto npos = ::compile_string::npos;

  explicit constexpr CompileString(char c) noexcept : container{0}
  {
    static_assert(N == 1, "Can only build CompileString<1> from char");
    this->container[0] = c;
  }
  explicit constexpr CompileString(char const (&contents)[N + 1]) noexcept
    : container{0}
  {
    for (auto i = std::size_t{0}; i < N + 1; ++i)
      this->container[i] = contents[i];
  }
  template <size_type B_N>
  constexpr CompileString(CompileString<B_N> const& src) noexcept : container{0}
  {
    constexpr auto end_pos = B_N > N ? N : B_N;
    for (auto i = size_type{0}; i < end_pos; ++i)
      this->container[i] = src[i];
    for (auto i = end_pos; i < N; ++i)
      this->container[i] = '\0';
  }
  constexpr CompileString(CompileString const& b) noexcept = default;
  constexpr CompileString(CompileString&& b) noexcept = default;
  ~CompileString() noexcept = default;

  constexpr CompileString& operator=(CompileString const& rhs) noexcept =
      default;
  constexpr CompileString& operator=(CompileString&& rhs) noexcept = default;

  constexpr operator std::string_view() const noexcept
  {
    return std::string_view{this->data(), N};
  }

  template <size_type B_N>
  constexpr bool operator==(CompileString<B_N> const& b) const noexcept
  {
    if constexpr (B_N == N)
      return std::equal(b.begin(), b.end(), this->begin());
    else
      return false;
  }
  template <size_type B_N>
  constexpr bool operator==(char const (&b)[B_N]) const noexcept
  {
    if constexpr (B_N - 1 == N)
      return std::equal(this->begin(), this->end(), &b[0]);
    return false;
  }
  template <size_type B_N>
  constexpr bool operator!=(CompileString<B_N> const& b) const noexcept
  {
    return !(*this == b);
  }
  template <size_type B_N>
  constexpr bool operator!=(char const (&b)[B_N]) const noexcept
  {
    return !(*this == b);
  }
  template <size_type B_N>
  constexpr bool operator<(CompileString<B_N> const& b) const noexcept
  {
    constexpr auto maxindex = N < B_N ? N : B_N;
    for (auto i = index_type{0}; i < maxindex; ++i)
    {
      if (this->container[i] < b[i])
        return true;
      else if (this->container[i] > b[i])
        return false;
    }
    return N < B_N;
  }
  template <size_type B_N>
  constexpr bool operator<(char const (&b)[B_N]) const noexcept
  {
    constexpr auto maxindex = N < B_N - 1 ? N : B_N - 1;
    for (auto i = index_type{0}; i < maxindex; ++i)
    {
      if (this->container[i] < b[i])
        return true;
      else if (this->container[i] > b[i])
        return false;
    }
    return N < B_N - 1;
  }
  template <size_type B_N>
  constexpr bool operator<=(CompileString<B_N> const& b) const noexcept
  {
    return *this < b || *this == b;
  }
  template <size_type B_N>
  constexpr bool operator<=(char const (&b)[B_N]) const noexcept
  {
    return *this < b || *this == b;
  }
  template <size_type B_N>
  constexpr bool operator>=(CompileString<B_N> const& b) const noexcept
  {
    return *this > b || *this == b;
  }
  template <size_type B_N>
  constexpr bool operator>=(char const (&b)[B_N]) const noexcept
  {
    return *this > b || *this == b;
  }
  template <size_type B_N>
  constexpr bool operator>(CompileString<B_N> const& b) const noexcept
  {
    return !(*this <= b);
  }
  template <size_type B_N>
  constexpr bool operator>(char const (&b)[B_N]) const noexcept
  {
    return !(*this <= b);
  }

  // Element access
  constexpr value_type& operator[](index_type idx) noexcept
  {
    return this->container[idx];
  }
  constexpr value_type const& operator[](index_type idx) const noexcept
  {
    return this->container[idx];
  }

  constexpr value_type& front() noexcept
  {
    return this->container[0];
  }
  constexpr value_type const& front() const noexcept
  {
    return this->container[0];
  }

  constexpr value_type& back() noexcept
  {
    return this->container[N - 1];
  }
  constexpr value_type const& back() const noexcept
  {
    return this->container[N - 1];
  }

  constexpr const_pointer data() const noexcept
  {
    return this->container.data();
  }
  constexpr const_pointer c_str() const noexcept
  {
    return this->data();
  }

  // Iterators
  constexpr iterator begin() noexcept
  {
    return this->container.begin();
  }
  constexpr const_iterator begin() const noexcept
  {
    return this->container.begin();
  }
  constexpr const_iterator cbegin() const noexcept
  {
    return this->container.cbegin();
  }

  constexpr iterator end() noexcept
  {
    return this->begin() + N;
  }
  constexpr const_iterator end() const noexcept
  {
    return this->begin() + N;
  }
  constexpr const_iterator cend() const noexcept
  {
    return this->cbegin() + N;
  }

  constexpr reverse_iterator rbegin() noexcept
  {
    return this->container.rbegin() + 1;
  }
  constexpr const_reverse_iterator rbegin() const noexcept
  {
    return this->container.rbegin() + 1;
  }
  constexpr const_reverse_iterator crbegin() const noexcept
  {
    return this->container.crbegin() + 1;
  }

  constexpr reverse_iterator rend() noexcept
  {
    return this->container.rend();
  }
  constexpr const_reverse_iterator rend() const noexcept
  {
    return this->container.rend();
  }
  constexpr const_reverse_iterator crend() const noexcept
  {
    return this->container.cend();
  }

  // Capacity
  constexpr size_type size() const noexcept
  {
    return N;
  }
  constexpr bool empty() const noexcept
  {
    return this->size() == 0;
  }

  // Operations
  template <size_type B_N>
  constexpr int compare(CompileString<B_N> const& b) const noexcept
  {
    if constexpr (N < B_N)
      return -b.compare(*this);
    else
    {
      for (auto i = index_type{0}; i < N; ++i)
        if (this->container[i] != b[i])
          return this->container[i] - b[i];
      return N > B_N ? 1 : 0;
    }
  }
  template <size_type B_N>
  constexpr int compare(char const (&b)[B_N]) const noexcept
  {
    return this->compare(CompileString<B_N - 1>{b});
  }

  template <size_type B_N>
  constexpr bool starts_with(char const (&b)[B_N]) const noexcept
  {
    if constexpr (N < B_N - 1)
      return false;
    else
      return std::equal(&b[0], &b[B_N - 1], this->container.data());
  }
  template <size_type B_N>
  constexpr bool starts_with(CompileString<B_N> const& b) const noexcept
  {
    if constexpr (N < B_N)
      return false;
    else
      return std::equal(b.begin(), b.end(), this->container.data());
  }

  template <size_type B_N>
  constexpr bool ends_with(char const (&b)[B_N]) const noexcept
  {
    if constexpr (N < B_N - 1)
      return false;
    else
      return std::equal(
          &b[0], &b[B_N - 1], this->container.begin() + N - (B_N - 1));
  }
  template <size_type B_N>
  constexpr bool ends_with(CompileString<B_N> const& b) const noexcept
  {
    if constexpr (N < B_N)
      return false;
    else
      return std::equal(b.begin(), b.end(), this->container.begin() + N - B_N);
  }

  // Search
  template <size_type B_N>
  constexpr index_type find(CompileString<B_N> const& b,
                            index_type pos = 0) const noexcept
  {
    if (B_N + pos > N)
      return npos;
    else
    {
      for (auto i = pos; i <= N - B_N; ++i)
        if (std::equal(b.begin(), b.end(), this->begin() + i))
          return i;
      return npos;
    }
  }
  template <size_type B_N>
  constexpr index_type find(char const (&b)[B_N], index_type pos = 0) const
      noexcept
  {
    if ((B_N - 1) + pos > N)
      return npos;
    else
    {
      for (auto i = pos; i <= N - (B_N - 1); ++i)
        if (std::equal(this->begin() + i, this->begin() + i + (B_N - 1), &b[0]))
          return i;
      return npos;
    }
  }
  constexpr index_type find(char c, index_type pos = 0) const noexcept
  {
    for (auto i = pos; i < N; ++i)
      if (this->container[i] == c)
        return i;
    return npos;
  }

  template <size_type B_N>
  constexpr index_type rfind(CompileString<B_N> const& b,
                             index_type pos = npos) const noexcept
  {
    auto start_pos = N - B_N < pos ? N - B_N : pos;
    for (auto i = start_pos; i > 0; --i)
      if (std::equal(b.begin(), b.end(), this->begin() + i))
        return i;
    return this->starts_with(b) ? 0 : npos;
  }
  template <size_type B_N>
  constexpr index_type rfind(char const (&b)[B_N], index_type pos = npos) const
      noexcept
  {
    auto start_pos = N - (B_N - 1) < pos ? N - (B_N - 1) : pos;
    for (auto i = start_pos; i > 0; --i)
      if (std::equal(&b[0], &b[B_N - 1], this->begin() + i))
        return i;
    return this->starts_with(b) ? 0 : npos;
  }
  constexpr index_type rfind(char c, index_type pos = npos) const noexcept
  {
    auto start_pos = N - 1 < pos ? N : pos + 1;
    for (auto i = start_pos; i > 0; --i)
      if (this->container[i - 1] == c)
        return i - 1;
    return npos;
  }

  template <size_type B_N>
  constexpr index_type find_first_of(CompileString<B_N> const& b,
                                     index_type pos = 0) const noexcept
  {
    for (auto i = pos; i < N; ++i)
      if (std::find(b.begin(), b.end(), this->container[i]) != b.end())
        return i;
    return npos;
  }
  template <size_type B_N>
  constexpr index_type find_first_of(char const (&b)[B_N],
                                     index_type pos = 0) const noexcept
  {
    for (auto i = pos; i < N; ++i)
      if (std::find(&b[0], &b[B_N - 1], this->container[i]) != &b[B_N - 1])
        return i;
    return npos;
  }
  constexpr index_type find_first_of(char c, index_type pos = 0) const noexcept
  {
    return this->find(c, pos);
  }

  template <size_type B_N>
  constexpr index_type find_first_not_of(CompileString<B_N> const& b,
                                         index_type pos = 0) const noexcept
  {
    for (auto i = pos; i < N; ++i)
      if (std::find(b.begin(), b.end(), this->container[i]) == b.end())
        return i;
    return npos;
  }
  template <size_type B_N>
  constexpr index_type find_first_not_of(char const (&b)[B_N],
                                         index_type pos = 0) const noexcept
  {
    for (auto i = pos; i < N; ++i)
      if (std::find(&b[0], &b[B_N - 1], this->container[i]) == &b[B_N - 1])
        return i;
    return npos;
  }
  constexpr index_type find_first_not_of(char c, index_type pos = 0) const
      noexcept
  {
    for (auto i = pos; i < N; ++i)
      if (this->container[i] != c)
        return i;
    return npos;
  }

  template <size_type B_N>
  constexpr index_type find_last_of(CompileString<B_N> const& b,
                                    index_type pos = npos) const noexcept
  {
    auto start_pos = pos > N - 1 ? N : pos + 1;
    for (auto i = start_pos; i > 0; --i)
      if (std::find(b.begin(), b.end(), this->container[i - 1]) != b.end())
        return i - 1;
    return npos;
  }
  template <size_type B_N>
  constexpr index_type find_last_of(char const (&b)[B_N],
                                    index_type pos = npos) const noexcept
  {
    auto start_pos = pos > N - 1 ? N : pos + 1;
    for (auto i = start_pos; i > 0; --i)
      if (std::find(&b[0], &b[B_N - 1], this->container[i - 1]) != &b[B_N - 1])
        return i - 1;
    return npos;
  }
  constexpr index_type find_last_of(char c, index_type pos = npos) const
      noexcept
  {
    return this->rfind(c, pos);
  }

  template <size_type B_N>
  constexpr index_type find_last_not_of(CompileString<B_N> const& b,
                                        index_type pos = npos) const noexcept
  {
    auto start_pos = pos > N - 1 ? N : pos + 1;
    for (auto i = start_pos; i > 0; --i)
      if (std::find(b.begin(), b.end(), this->container[i - 1]) == b.end())
        return i - 1;
    return npos;
  }
  template <size_type B_N>
  constexpr index_type find_last_not_of(char const (&b)[B_N],
                                        index_type pos = npos) const noexcept
  {
    auto start_pos = pos > N - 1 ? N : pos + 1;
    for (auto i = start_pos; i > 0; --i)
      if (std::find(&b[0], &b[B_N - 1], this->container[i - 1]) == &b[B_N - 1])
        return i - 1;
    return npos;
  }
  constexpr index_type find_last_not_of(char c, index_type pos = npos) const
      noexcept
  {
    auto start_pos = pos > N - 1 ? N : pos + 1;
    for (auto i = start_pos; i > 0; --i)
      if (this->container[i - 1] != c)
        return i - 1;
    return npos;
  }

  // operator+
  constexpr auto operator+(char c) const noexcept
  {
    auto ret = CompileString<N + 1>{*this};
    ret[N] = c;
    return ret;
  }
  template <size_type B_N>
  constexpr auto operator+(CompileString<B_N> const& b) const noexcept
  {
    auto ret = CompileString<N + B_N>{*this};
    for (auto i = size_type{0}; i < B_N; ++i)
      ret[i + N] = b[i];
    return ret;
  }
  template <size_type B_N>
  constexpr auto operator+(char const (&b)[B_N]) const noexcept
  {
    auto ret = CompileString<N + B_N - 1>{*this};
    for (auto i = size_type{0}; i < B_N - 1; ++i)
      ret[i + N] = b[i];
    return ret;
  }

private:
  container_type container;
};

template <std::size_t N>
CompileString(char const (&s)[N])->CompileString<N - 1>;
CompileString(char c)->CompileString<1>;

template <std::size_t N, std::size_t B_N>
inline constexpr bool operator==(char const (&a)[N],
                                 CompileString<B_N> const& b) noexcept
{
  return b == a;
}
template <std::size_t N, std::size_t B_N>
inline constexpr bool operator!=(char const (&a)[N],
                                 CompileString<B_N> const& b) noexcept
{
  return b != a;
}
template <std::size_t N, std::size_t B_N>
inline constexpr bool operator<(char const (&a)[N],
                                CompileString<B_N> const& b) noexcept
{
  return b > a;
}
template <std::size_t N, std::size_t B_N>
inline constexpr bool operator>(char const (&a)[N],
                                CompileString<B_N> const& b) noexcept
{
  return b < a;
}
template <std::size_t N, std::size_t B_N>
inline constexpr bool operator<=(char const (&a)[N],
                                 CompileString<B_N> const& b) noexcept
{
  return b >= a;
}
template <std::size_t N, std::size_t B_N>
inline constexpr bool operator>=(char const (&a)[N],
                                 CompileString<B_N> const& b) noexcept
{
  return b <= a;
}

template <std::size_t N, std::size_t B_N>
inline constexpr auto operator+(char const (&a)[N],
                                CompileString<B_N> const& b) noexcept
{
  return CompileString{a} + b;
}
template <std::size_t B_N>
inline constexpr auto operator+(char a, CompileString<B_N> const& b) noexcept
{
  return CompileString{a} + b;
}
}

#endif /* !COMPILESTRING_COMPILESTRING_HPP_ */
