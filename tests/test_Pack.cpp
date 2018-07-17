#include <mysql_orm/meta/Pack.hpp>

#include <type_traits>

#include <catch.hpp>

using mysql_orm::meta::AppendPack_t;
using mysql_orm::meta::AppendValuePack_t;
using mysql_orm::meta::FilterPack_t;
using mysql_orm::meta::FilterValuePack_t;
using mysql_orm::meta::MergePacks_t;
using mysql_orm::meta::MergeValuePacks_t;
using mysql_orm::meta::Pack;
using mysql_orm::meta::PackContains_v;
using mysql_orm::meta::PrependPack_t;
using mysql_orm::meta::PrependValuePack_t;
using mysql_orm::meta::ValuePack;
using mysql_orm::meta::ValuePackContains_v;

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

TEST_CASE("[Pack] Contains", "[Pack]")
{
  CHECK(PackContains_v<int, Pack<int>>);
  CHECK(!PackContains_v<int, Pack<>>);
  CHECK(PackContains_v<int, Pack<int, int>>);
  CHECK(PackContains_v<int, Pack<int, double>>);
  CHECK(!PackContains_v<int, Pack<double>>);
  CHECK(!PackContains_v<int, Pack<double, char, float, double>>);
  CHECK(PackContains_v<int, Pack<double, char, float, double, int>>);
}

TEST_CASE("[ValuePack] ValuePack size", "[ValuePack]")
{
  CHECK(ValuePack<1, 4, 5>::size == 3);
  CHECK(ValuePack<'c'>::size == 1);
  CHECK(ValuePack<>::size == 0);
}

TEST_CASE("[ValuePack] Merge", "[ValuePack]")
{
  CHECK(std::is_same_v<
        MergeValuePacks_t<ValuePack<1, 2, 3>, ValuePack<4, 'c', 1>>,
        ValuePack<1, 2, 3, 4, 'c', 1>>);
  CHECK(std::is_same_v<MergeValuePacks_t<ValuePack<1, 2, 3>, ValuePack<>>,
                       ValuePack<1, 2, 3>>);
  CHECK(std::is_same_v<MergeValuePacks_t<ValuePack<>, ValuePack<4, 'c', 1>>,
                       ValuePack<4, 'c', 1>>);
  CHECK(std::is_same_v<MergeValuePacks_t<ValuePack<1>, ValuePack<'c'>>,
                       ValuePack<1, 'c'>>);
}

TEST_CASE("[ValuePack] Append", "[ValuePack]")
{
  CHECK(std::is_same_v<AppendValuePack_t<ValuePack<1, 2>, 'c'>,
                       ValuePack<1, 2, 'c'>>);
  CHECK(std::is_same_v<AppendValuePack_t<ValuePack<>, 'c'>, ValuePack<'c'>>);
}

TEST_CASE("[ValuePack] Prepend", "[ValuePack]")
{
  CHECK(std::is_same_v<PrependValuePack_t<ValuePack<1, 2>, 'c'>,
                       ValuePack<'c', 1, 2>>);
  CHECK(std::is_same_v<PrependValuePack_t<ValuePack<>, 'c'>, ValuePack<'c'>>);
}

namespace
{
template <auto X>
struct IsEven : std::conditional_t<X % 2 == 0, std::true_type, std::false_type>
{
};
}

TEST_CASE("[ValuePack] Filter", "[ValuePack]")
{
  CHECK(std::is_same_v<
        FilterValuePack_t<IsEven,
                          ValuePack<1, 2, 3, 4, 5, 6, 8, 12, 127, 65536>>,
        ValuePack<2, 4, 6, 8, 12, 65536>>);
  CHECK(std::is_same_v<
        FilterValuePack_t<IsEven, ValuePack<1, 3, 5, 7, 11, 127, 65535>>,
        ValuePack<>>);
}

TEST_CASE("[ValuePack] Contains", "[ValuePack]")
{
  CHECK(ValuePackContains_v<1, ValuePack<1>>);
  CHECK(!ValuePackContains_v<1, ValuePack<>>);
  CHECK(ValuePackContains_v<1, ValuePack<1, 1>>);
  CHECK(ValuePackContains_v<1, ValuePack<1, 2>>);
  CHECK(!ValuePackContains_v<1, ValuePack<2>>);
  CHECK(!ValuePackContains_v<1, ValuePack<2, 3, 4, 2>>);
  CHECK(ValuePackContains_v<1, ValuePack<2, 3, 4, 2, 1>>);
}
