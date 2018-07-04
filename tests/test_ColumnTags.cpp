#include <mysql_orm/ColumnConstraints.hpp>

#include <catch.hpp>

using namespace mysql_orm;

TEST_CASE("Constraints creation", "[ColumnConstraints]")
{
  SECTION("Default")
  {
    constexpr auto t = ColumnConstraints{};
    CHECK(t.nullable == Tristate::Undefined);
    CHECK(t.auto_increment == false);
    CHECK(t.primary_key == false);
    CHECK(t.unique == false);
  }

  SECTION("All set")
  {
    constexpr auto t =
        ColumnConstraints{NotNull{}, PrimaryKey{}, Autoincrement{}, Unique{}};
    CHECK(t.nullable == Tristate::Off);
    CHECK(t.auto_increment == true);
    CHECK(t.primary_key == true);
    CHECK(t.unique == true);
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
