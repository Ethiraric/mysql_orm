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
  }

  SECTION("All set")
  {
    constexpr auto t =
        ColumnConstraints{NotNull{}, PrimaryKey{}, Autoincrement{}};
    CHECK(t.nullable == Tristate::Off);
    CHECK(t.auto_increment == true);
    CHECK(t.primary_key == true);
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
