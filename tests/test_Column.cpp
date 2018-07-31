#include <mysql_orm/Column.hpp>

#include <optional>

#include <catch.hpp>

#include <Record.hh>

using PrimaryKey = mysql_orm::PrimaryKey;
using Autoincrement = mysql_orm::Autoincrement;
using NotNull = mysql_orm::NotNull;
using Nullable = mysql_orm::Nullable;
using Unique = mysql_orm::Unique;

TEST_CASE("Create fields", "[Column]")
{
  SECTION("String")
  {
    constexpr auto c = mysql_orm::make_column<&Record::s>("s");
    constexpr auto sch = c.getSchema();
    REQUIRE(sch == "`s` TEXT NOT NULL");
  }

  SECTION("Integers")
  {
    SECTION("int")
    {
      constexpr auto c = mysql_orm::make_column<&Record::i>("i");
      REQUIRE(c.getSchema() == "`i` INTEGER NOT NULL");
    }
    SECTION("mysql_orm::id_t")
    {
      constexpr auto c = mysql_orm::make_column<&Record::id>("id");
      REQUIRE(c.getSchema() == "`id` INTEGER UNSIGNED NOT NULL");
    }
  }

  SECTION("Optionals")
  {
    SECTION("String")
    {
      constexpr auto c = mysql_orm::make_column<&RecordWithOptionals::s>("s");
      REQUIRE(c.getSchema() == "`s` TEXT");
    }

    SECTION("Integers")
    {
      SECTION("int")
      {
        constexpr auto c = mysql_orm::make_column<&RecordWithOptionals::i>("i");
        REQUIRE(c.getSchema() == "`i` INTEGER");
      }
      SECTION("mysql_orm::id_t")
      {
        constexpr auto c =
            mysql_orm::make_column<&RecordWithOptionals::id>("id");
        REQUIRE(c.getSchema() == "`id` INTEGER UNSIGNED");
      }
    }
  }

  SECTION("With attributes")
  {
    SECTION("Primary key auto_increment")
    {
      constexpr auto c = mysql_orm::make_column<&MixedRecord::id>(
          "id", PrimaryKey{}, Autoincrement{});
      REQUIRE(c.getSchema() ==
              "`id` INTEGER UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT");
    }

    SECTION("Unique")
    {
      constexpr auto c =
          mysql_orm::make_column<&MixedRecord::id>("id", Unique{});
      REQUIRE(c.getSchema() == "`id` INTEGER UNSIGNED UNIQUE NOT NULL");
    }

    SECTION("Nullable non-optional")
    {
      constexpr auto c =
          mysql_orm::make_column<&MixedRecord::i>("i", Nullable{});
      REQUIRE(c.getSchema() == "`i` INTEGER");
    }

    SECTION("Non-null optional")
    {
      constexpr auto c =
          mysql_orm::make_column<&MixedRecord::s>("foo", NotNull{});
      REQUIRE(c.getSchema() == "`foo` TEXT NOT NULL");
    }
  }
}
