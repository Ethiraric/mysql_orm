#include <mysql_orm/meta/RemoveOccurences.hpp>

#include <catch_amalgamated.hpp>

using mysql_orm::meta::Pack;
using mysql_orm::meta::RemoveOccurences_t;
using mysql_orm::meta::RemoveValueOccurences_t;
using mysql_orm::meta::ValuePack;

TEST_CASE("[RemoveOccurences] No needle", "[RemoveOccurences]")
{
  CHECK(std::is_same_v<RemoveOccurences_t<Pack<int, double, float>, Pack<>>,
                       Pack<int, double, float>>);
}

TEST_CASE("[RemoveOccurences] One needle", "[RemoveOccurences]")
{
  CHECK(std::is_same_v<
        RemoveOccurences_t<Pack<int, double, int, char, float, int>, Pack<int>>,
        Pack<double, char, float>>);
  CHECK(std::is_same_v<
        RemoveOccurences_t<Pack<int, double, int, char, float, int>,
                           Pack<long>>,
        Pack<int, double, int, char, float, int>>);
}

TEST_CASE("[RemoveOccurences] Several needles", "[RemoveOccurences]")
{
  CHECK(std::is_same_v<
        RemoveOccurences_t<Pack<int, double, int, char, float, char, int>,
                           Pack<int, char>>,
        Pack<double, float>>);
  CHECK(std::is_same_v<
        RemoveOccurences_t<Pack<int, double, int, char, float, int>,
                           Pack<long, int>>,
        Pack<double, char, float>>);
}

TEST_CASE("[RemoveValueOccurences] No needle", "[RemoveValueOccurences]")
{
  CHECK(std::is_same_v<RemoveValueOccurences_t<ValuePack<1, 2, 3>, ValuePack<>>,
                       ValuePack<1, 2, 3>>);
}

TEST_CASE("[RemoveValueOccurences] One needle", "[RemoveValueOccurences]")
{
  CHECK(std::is_same_v<
        RemoveValueOccurences_t<ValuePack<1, 2, 1, 3, 4, 1>, ValuePack<1>>,
        ValuePack<2, 3, 4>>);
  CHECK(std::is_same_v<
        RemoveValueOccurences_t<ValuePack<1, 2, 1, 3, 4, 1>, ValuePack<5>>,
        ValuePack<1, 2, 1, 3, 4, 1>>);
}

TEST_CASE("[RemoveValueOccurences] Several needles", "[RemoveValueOccurences]")
{
  CHECK(std::is_same_v<RemoveValueOccurences_t<ValuePack<1, 2, 1, 3, 4, 3, 1>,
                                               ValuePack<1, 3>>,
                       ValuePack<2, 4>>);
  CHECK(std::is_same_v<
        RemoveValueOccurences_t<ValuePack<1, 2, 1, 3, 4, 1>, ValuePack<5, 1>>,
        ValuePack<2, 3, 4>>);
}
