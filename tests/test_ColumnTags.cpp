#include <mysql_orm/ColumnConstraints.hpp>

#include <catch.hpp>

using namespace mysql_orm;

TEST_CASE("Constraints creation", "[ColumnConstraints]")
{
  SECTION("Default")
  {
    constexpr auto c = columnConstraintsFromPack(meta::Pack<>{});
    CHECK(c.nullable() == Tristate::Undefined);
    CHECK(c.auto_increment() == false);
    CHECK(c.primary_key() == false);
    CHECK(c.unique() == false);
  }

  SECTION("All set")
  {
    constexpr auto c = columnConstraintsFromPack(
        meta::Pack<NotNull, PrimaryKey, Autoincrement, Unique>{});
    CHECK(c.nullable() == Tristate::Off);
    CHECK(c.auto_increment() == true);
    CHECK(c.primary_key() == true);
    CHECK(c.unique() == true);
  }

  // Should not compile.
  // SECTION("Duplicate")
  // {
  //   constexpr auto t = ColumnConstraints{NotNull{}, NotNull{}};
  //   CHECK(t.nullable == Tristate::Off);
  //   CHECK(t.auto_increment == true);
  //   CHECK(t.primary_key == true);
  // }
}
