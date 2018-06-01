#include <mysql_orm/Table.hpp>

#include <catch.hpp>

#include <string>

#include <Record.hh>

using mysql_orm::make_column;
using mysql_orm::make_table;

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
