#include <mysql_orm/GetAll.hpp>

#include <catch_amalgamated.hpp>

#include <Record.hh>
#include <mysql_orm/Database.hpp>
#include <mysql_orm/Where.hpp>

using mysql_orm::c;
using mysql_orm::Connection;
using mysql_orm::Limit;
using mysql_orm::make_column;
using mysql_orm::make_database;
using mysql_orm::make_table;
using mysql_orm::MySQLException;
using mysql_orm::ref;
using mysql_orm::Where;

TEST_CASE("[GetAll] GetAll buildquery", "[GetAll]")
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

  CHECK(d.getAll<MixedRecord>().buildquery() ==
        "SELECT `id`, `i`, `foo` FROM `mixed_records`");
}

TEST_CASE("[GetAll] GetAll", "[GetAll]")
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

  SECTION("One row")
  {
    d.execute("DELETE FROM `records` WHERE `id`>1");
    auto const res = d.getAll<Record>().build().execute();
    static_assert(
        std::is_same_v<std::remove_cv_t<decltype(res)>, std::vector<Record>>,
        "Wrong return type");
    REQUIRE(res.size() == 1);
    CHECK(res[0] == Record{1, 1, "one"});
  }

  SECTION("All rows")
  {
    auto const res = d.getAll<Record>().build().execute();
    static_assert(
        std::is_same_v<std::remove_cv_t<decltype(res)>, std::vector<Record>>,
        "Wrong return type");
    REQUIRE(res.size() == 3);
    CHECK(res[0] == Record{1, 1, "one"});
    CHECK(res[1] == Record{2, 2, "two"});
    CHECK(res[2] == Record{3, 4, "four"});
  }
}

TEST_CASE("[GetAll] GetAll with optionals", "[GetAll]")
{
  auto table_records = make_table("optional_records",
                                  make_column<&RecordWithOptionals::id>("id"),
                                  make_column<&RecordWithOptionals::i>("i"),
                                  make_column<&RecordWithOptionals::s>("s"));
  auto table_records_with_time =
      make_table("records_with_time",
                 make_column<&RecordWithTime::id>("id"),
                 make_column<&RecordWithTime::time>("time"));
  auto connection =
      Connection{"localhost", 3306, "mysql_orm_test", "", "mysql_orm_test_db"};
  auto d = make_database(connection, table_records_with_time, table_records);

  d.recreate();
  d.execute(
      "INSERT INTO `optional_records` (`id`, `i`, `s`) VALUES "
      R"((1, 1, "one"),)"
      R"((2, 2, "two"),)"
      R"((3, 4, "four"))");
  d.execute(
      "INSERT INTO `records_with_time` (`id`, `time`) VALUES (1, '2018-01-02 "
      "03:04:05')");

  SECTION("One row")
  {
    d.execute("DELETE FROM `optional_records` WHERE `id`>1");
    auto const res = d.getAll<RecordWithOptionals>().build().execute();
    static_assert(std::is_same_v<std::remove_cv_t<decltype(res)>,
                                 std::vector<RecordWithOptionals>>,
                  "Wrong return type");
    REQUIRE(res.size() == 1);
    CHECK(res[0] == RecordWithOptionals{1, 1, "one"});
  }

  SECTION("All rows")
  {
    auto const res = d.getAll<RecordWithOptionals>().build().execute();
    static_assert(std::is_same_v<std::remove_cv_t<decltype(res)>,
                                 std::vector<RecordWithOptionals>>,
                  "Wrong return type");
    REQUIRE(res.size() == 3);
    CHECK(res[0] == RecordWithOptionals{1, 1, "one"});
    CHECK(res[1] == RecordWithOptionals{2, 2, "two"});
    CHECK(res[2] == RecordWithOptionals{3, 4, "four"});
  }

  SECTION("With time")
  {
    auto const res = d.getAll<RecordWithTime>()();
    static_assert(std::is_same_v<std::remove_cv_t<decltype(res)>,
                                 std::vector<RecordWithTime>>,
                  "Wrong return type");
    REQUIRE(res.size() == 1);
    CHECK(res[0] == RecordWithTime{1, makeTm(2018, 1, 2, 3, 4, 5)});
  }
}
