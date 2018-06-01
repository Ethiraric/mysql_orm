#include <mysql_orm/ColumnTags.hpp>

#include <catch.hpp>

using namespace mysql_orm;

TEST_CASE("Tags creation", "[ColumnTags]")
{
  SECTION("Default")
  {
    constexpr auto t = ColumnTags{};
    CHECK(t.nullable == Tristate::Undefined);
    CHECK(t.auto_increment == false);
    CHECK(t.primary_key == false);
  }

  SECTION("All set")
  {
    constexpr auto t = ColumnTags{NotNull{}, PrimaryKey{}, Autoincrement{}};
    CHECK(t.nullable == Tristate::Off);
    CHECK(t.auto_increment == true);
    CHECK(t.primary_key == true);
  }

  // Should not compile.
  // SECTION("Duplicate")
  // {
  //   constexpr auto t = ColumnTags{NotNull{}, NotNull{}};
  //   CHECK(t.nullable == Tristate::Off);
  //   CHECK(t.auto_increment == true);
  //   CHECK(t.primary_key == true);
  // }
}
