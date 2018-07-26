#include <mysql_orm/Delete.hpp>

#include <catch.hpp>

#include <Record.hh>
#include <mysql_orm/Database.hpp>
#include <mysql_orm/Where.hpp>

using mysql_orm::c;
using mysql_orm::Connection;
using mysql_orm::Limit;
using mysql_orm::make_column;
using mysql_orm::make_database;
using mysql_orm::make_table;
using mysql_orm::Where;

TEST_CASE("[Delete] Delete buildquery", "[Delete]")
{
  auto table_mixed_records = make_table("mixed_records",
                                        make_column<&MixedRecord::id>("id"),
                                        make_column<&MixedRecord::i>("i"),
                                        make_column<&MixedRecord::s>("foo"));
  auto table_records = make_table("records",
                                  make_column<&Record::id>("id"),
                                  make_column<&Record::i>("i"),
                                  make_column<&Record::s>("s"));
  auto connection =
      Connection{"localhost", 3306, "mysql_orm_test", "", "mysql_orm_test_db"};
  auto d = make_database(connection, table_mixed_records, table_records);

  CHECK(d.delete_<MixedRecord>().buildquery() ==
        "DELETE FROM `mixed_records`");
}

TEST_CASE("[Delete] Delete", "[Delete]")
{
  auto table_records = make_table("records",
                                  make_column<&Record::id>("id"),
                                  make_column<&Record::i>("i"),
                                  make_column<&Record::s>("s"));
  auto connection =
      Connection{"localhost", 3306, "mysql_orm_test", "", "mysql_orm_test_db"};
  auto d = make_database(connection, table_records);

  d.recreate();
  d.execute(
      "INSERT INTO `records` (`id`, `i`, `s`) VALUES "
      R"((1, 1, "one"),)"
      R"((2, 2, "two"),)"
      R"((3, 4, "four"))");

  SECTION("Everything")
  {
    d.delete_<Record>()();
    auto const res = d.getAll<Record>()();
    REQUIRE(res.empty());
  }

  SECTION("Limit")
  {
    d.delete_<Record>()(Limit<1>{})();
    auto const res = d.getAll<Record>()();
    REQUIRE(res.size() == 2);
  }

  SECTION("Where")
  {
    d.delete_<Record>()(Where{c<&Record::s>{} == "four"})();
    auto const res = d.getAll<Record>()();
    REQUIRE(res.size() == 2);
    CHECK(res[0] == Record{1, 1, "one"});
    CHECK(res[1] == Record{2, 2, "two"});
  }
}
