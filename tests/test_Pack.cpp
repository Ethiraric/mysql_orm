#include <mysql_orm/meta/Pack.hpp>

#include <type_traits>

#include <catch.hpp>

using mysql_orm::meta::AppendPack_t;
using mysql_orm::meta::FilterPack_t;
using mysql_orm::meta::MergePacks_t;
using mysql_orm::meta::Pack;
using mysql_orm::meta::PrependPack_t;

TEST_CASE("[Pack] Pack size", "[Pack]")
{
  CHECK(Pack<int, int, float>::size == 3);
  CHECK(Pack<Pack<>>::size == 1);
  CHECK(Pack<>::size == 0);
}

TEST_CASE("[Pack] Merge", "[Pack]")
{
  CHECK(std::is_same_v<
        MergePacks_t<Pack<int, float, char>, Pack<char, float, int>>,
        Pack<int, float, char, char, float, int>>);
  CHECK(std::is_same_v<MergePacks_t<Pack<int, float, char>, Pack<>>,
                       Pack<int, float, char>>);
  CHECK(std::is_same_v<MergePacks_t<Pack<>, Pack<char, float, int>>,
                       Pack<char, float, int>>);
  CHECK(std::is_same_v<MergePacks_t<Pack<int>, Pack<int>>, Pack<int, int>>);
}

TEST_CASE("[Pack] Append", "[Pack]")
{
  CHECK(std::is_same_v<AppendPack_t<Pack<int, float>, char>,
                       Pack<int, float, char>>);
  CHECK(std::is_same_v<AppendPack_t<Pack<int, float>, Pack<char, double>>,
                       Pack<int, float, Pack<char, double>>>);
}

TEST_CASE("[Pack] Prepend", "[Pack]")
{
  CHECK(std::is_same_v<PrependPack_t<Pack<int, float>, char>,
                       Pack<char, int, float>>);
  CHECK(std::is_same_v<PrependPack_t<Pack<int, float>, Pack<char, double>>,
                       Pack<Pack<char, double>, int, float>>);
}

TEST_CASE("[Pack] Filter", "[Pack]")
{
  CHECK(
      std::is_same_v<FilterPack_t<std::is_integral,
                                  Pack<int, float, double, char, unsigned int>>,
                     Pack<int, char, unsigned int>>);
  CHECK(std::is_same_v<
        FilterPack_t<std::is_integral,
                     Pack<std::string, float, double, Pack<>, Pack<int>>>,
        Pack<>>);
}
