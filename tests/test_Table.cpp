#include <mysql_orm/Table.hpp>

#include <catch.hpp>

#include <string>

#include <Record.hh>
#include <Utils.hh>

using mysql_orm::c;
using mysql_orm::make_column;
using mysql_orm::make_table;
using mysql_orm::Where;

TEST_CASE("Create statement", "[Table]")
{
  SECTION("Simple record")
  {
    auto t = make_table("record",
                        make_column<&MixedRecord::id>("id"),
                        make_column<&MixedRecord::i>("i"),
                        make_column<&MixedRecord::s>("foo"));
    CHECK(t.getSchema() ==
          "CREATE TABLE `record` (\n"
          "  `id` INTEGER UNSIGNED NOT NULL,\n"
          "  `i` INTEGER NOT NULL,\n"
          "  `foo` TEXT\n"
          ")");
  }

  // Should not compile
  // SECTION("Mismatched records")
  // {
  //   auto t = make_table("record",
  //                       make_column<&MixedRecord::id>("id"),
  //                       make_column<&MixedRecord::i>("i"),
  //                       make_column<&Record::s>("foo"));
  // }
}

TEST_CASE("Selection statement", "[Table]")
{
  SECTION("Simple record")
  {
    SECTION("Select all")
    {
      auto t = make_table("record",
                          make_column<&MixedRecord::id>("id"),
                          make_column<&MixedRecord::i>("i"),
                          make_column<&MixedRecord::s>("foo"));
      CHECK(t.select(dangling_ref<MYSQL>()).buildquery() ==
            "SELECT `id`, `i`, `foo` "
            "FROM `record`");
    }

    SECTION("Select some fields")
    {
      auto t = make_table("record",
                          make_column<&MixedRecord::id>("id"),
                          make_column<&MixedRecord::i>("i"),
                          make_column<&MixedRecord::s>("foo"));
      CHECK(t.select<&MixedRecord::id, &MixedRecord::s>(dangling_ref<MYSQL>())
                .buildquery() ==
            "SELECT `id`, `foo` "
            "FROM `record`");
    }
  }
}

TEST_CASE("Where clause", "[Table]")
{
  SECTION("Simple record")
  {
    SECTION("Where id=1")
    {
      auto t = make_table("record",
                          make_column<&MixedRecord::id>("id"),
                          make_column<&MixedRecord::i>("i"),
                          make_column<&MixedRecord::s>("foo"));
      CHECK(t.select(dangling_ref<MYSQL>())(Where{c<&MixedRecord::id>{} == 1})
                .buildquery() ==
            "SELECT `id`, `i`, `foo` "
            "FROM `record` WHERE `id`=1");
    }
  }
}
