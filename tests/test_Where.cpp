#include <mysql_orm/Where.hpp>

#include <catch.hpp>

#include <Record.hh>
#include <mysql_orm/Database.hpp>
#include <mysql_orm/GetAll.hpp>

using mysql_orm::c;
using mysql_orm::Connection;
using mysql_orm::make_column;
using mysql_orm::ref;
using mysql_orm::Where;

TEST_CASE("[Where] Where buildquery", "[Where]")
{
  auto table_records = make_table("records",
                                  make_column<&Record::id>("id"),
                                  make_column<&Record::i>("i"),
                                  make_column<&Record::s>("s"));
  auto connection =
      Connection{"localhost", 3306, "mysql_orm_test", "", "mysql_orm_test_db"};
  auto d = make_database(connection, table_records);

  CHECK(d.getAll<Record>()(Where{c<&Record::i>{} == 3}).buildquery() ==
        "SELECT `id`, `i`, `s` FROM `records` WHERE `i`=?");
  SECTION("Where query")
  {
    auto const res =
        d.getAll<Record>()(Where{c<&Record::i>{} == 4}).build().execute();
    static_assert(
        std::is_same_v<std::remove_cv_t<decltype(res)>, std::vector<Record>>,
        "Wrong return type");
    REQUIRE(res.size() == 1);
    CHECK(res[0] == Record{3, 4, "four"});
  }
}

TEST_CASE("[Where] Simple Where", "[Where]")
{
  auto table_records = make_table("records",
                                  make_column<&Record::id>("id"),
                                  make_column<&Record::i>("i"),
                                  make_column<&Record::s>("s"));
  auto table_records_with_time =
      make_table("records_with_time",
                 make_column<&RecordWithTime::id>("id"),
                 make_column<&RecordWithTime::time>("time"));
  auto connection =
      Connection{"localhost", 3306, "mysql_orm_test", "", "mysql_orm_test_db"};
  auto d = make_database(connection, table_records, table_records_with_time);

  d.recreate();
  d.execute(
      "INSERT INTO `records` (`id`, `i`, `s`) VALUES "
      R"((1, 1, "one"),)"
      R"((2, 2, "two"),)"
      R"((3, 4, "four"))");
  d.execute(
      "INSERT INTO `records_with_time` (`id`, `time`) VALUES "
      "(1, '2018-01-02 03:04:05'), "
      "(2, '2018-03-02 03:04:05')");

  SECTION("With value")
  {
    auto const res =
        d.getAll<Record>()(Where{c<&Record::i>{} == 4}).build().execute();
    static_assert(
        std::is_same_v<std::remove_cv_t<decltype(res)>, std::vector<Record>>,
        "Wrong return type");
    REQUIRE(res.size() == 1);
    CHECK(res[0] == Record{3, 4, "four"});
  }

  SECTION("With variable")
  {
    auto const i = 4;
    auto const res =
        d.getAll<Record>()(Where{c<&Record::i>{} == i}).build().execute();
    static_assert(
        std::is_same_v<std::remove_cv_t<decltype(res)>, std::vector<Record>>,
        "Wrong return type");
    REQUIRE(res.size() == 1);
    CHECK(res[0] == Record{3, 4, "four"});
  }

  SECTION("With reference")
  {
    auto i = 3;
    auto const query = d.getAll<Record>()(Where{c<&Record::i>{} == ref{i}});
    i = 4;
    auto const res = query.build().execute();
    static_assert(
        std::is_same_v<std::remove_cv_t<decltype(res)>, std::vector<Record>>,
        "Wrong return type");
    REQUIRE(res.size() == 1);
    CHECK(res[0] == Record{3, 4, "four"});
  }

  SECTION("Logic operator in condition")
  {
    d.execute(
        "INSERT INTO `records` (`id`, `i`, `s`) VALUES "
        R"((4, 1, "one"))");
    auto const res =
        d.getAll<Record>()(Where{c<&Record::i>{} == 1 && c<&Record::id>{} == 4})
            .build()
            .execute();
    static_assert(
        std::is_same_v<std::remove_cv_t<decltype(res)>, std::vector<Record>>,
        "Wrong return type");
    REQUIRE(res.size() == 1);
    CHECK(res[0] == Record{4, 1, "one"});
  }

  SECTION("Raw char const* string")
  {
    auto const res =
        d.getAll<Record>()(Where{c<&Record::s>{} == "one"}).build().execute();
    static_assert(
        std::is_same_v<std::remove_cv_t<decltype(res)>, std::vector<Record>>,
        "Wrong return type");
    REQUIRE(res.size() == 1);
    CHECK(res[0] == Record{1, 1, "one"});
  }

  SECTION("With time")
  {
    auto const res = d.getAll<RecordWithTime>()(
        Where{c<&RecordWithTime::time>{} == makeTm(2018, 1, 2, 3, 4, 5)})();
    REQUIRE(res.size() == 1);
    CHECK(res[0] == RecordWithTime{1, makeTm(2018, 1, 2, 3, 4, 5)});
  }

  SECTION("With time reference")
  {
    auto time = makeTm(2018, 1, 2, 3, 4, 5);
    auto query = d.getAll<RecordWithTime>()(
        Where{c<&RecordWithTime::time>{} == ref{time}});
    time = makeTm(2018, 3, 2, 3, 4, 5);
    auto const res = query();
    REQUIRE(res.size() == 1);
    CHECK(res[0] == RecordWithTime{2, time});
  }
}
