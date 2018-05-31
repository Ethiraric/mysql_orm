#include <mysql_orm/Column.hpp>

#include <optional>

#include <catch.hpp>

namespace
{
struct Record
{
  mysql_orm::id_t id;
  int i;
  std::string s;
};

struct RecordWithOptionals
{
  std::optional<mysql_orm::id_t> id;
  std::optional<int> i;
  std::optional<std::string> s;
};
}

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
}
