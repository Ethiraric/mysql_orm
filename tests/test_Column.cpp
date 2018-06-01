#include <mysql_orm/Column.hpp>

#include <optional>

#include <catch.hpp>

#include <Record.hh>

using PrimaryKey = mysql_orm::PrimaryKey;
using Autoincrement = mysql_orm::Autoincrement;
using NotNull = mysql_orm::NotNull;
using Nullable = mysql_orm::Nullable;

TEST_CASE("Create fields", "[Column]")
{
  SECTION("String")
  {
    auto c = mysql_orm::make_column<&Record::s>("s");
    REQUIRE(c.getSchema() == "`s` TEXT NOT NULL");
  }

  SECTION("Integers")
  {
    SECTION("int")
    {
      auto c = mysql_orm::make_column<&Record::i>("i");
      REQUIRE(c.getSchema() == "`i` INTEGER NOT NULL");
    }
    SECTION("mysql_orm::id_t")
    {
      auto c = mysql_orm::make_column<&Record::id>("id");
      REQUIRE(c.getSchema() == "`id` INTEGER UNSIGNED NOT NULL");
    }
  }

  SECTION("Optionals")
  {
    SECTION("String")
    {
      auto c = mysql_orm::make_column<&RecordWithOptionals::s>("s");
      REQUIRE(c.getSchema() == "`s` TEXT");
    }

    SECTION("Integers")
    {
      SECTION("int")
      {
        auto c = mysql_orm::make_column<&RecordWithOptionals::i>("i");
        REQUIRE(c.getSchema() == "`i` INTEGER");
      }
      SECTION("mysql_orm::id_t")
      {
        auto c = mysql_orm::make_column<&RecordWithOptionals::id>("id");
        REQUIRE(c.getSchema() == "`id` INTEGER UNSIGNED");
      }
    }
  }

  SECTION("With attributes")
  {
    SECTION("Primary key auto_increment")
    {
      auto c = mysql_orm::make_column<&MixedRecord::id>(
          "id", PrimaryKey{}, Autoincrement{});
      REQUIRE(c.getSchema() ==
              "`id` INTEGER UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT");
    }

    SECTION("Nullable non-optional")
    {
      auto c = mysql_orm::make_column<&MixedRecord::i>("i", Nullable{});
      REQUIRE(c.getSchema() == "`i` INTEGER");
    }

    SECTION("Non-null optional")
    {
      auto c = mysql_orm::make_column<&MixedRecord::s>("foo", NotNull{});
      REQUIRE(c.getSchema() == "`foo` TEXT NOT NULL");
    }
  }
}
