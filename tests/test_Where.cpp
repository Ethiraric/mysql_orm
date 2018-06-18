#include <mysql_orm/Where.hpp>

#include <catch.hpp>

#include <Record.hh>
#include <mysql_orm/Database.hpp>
#include <mysql_orm/Select.hpp>

using mysql_orm::c;
using mysql_orm::make_column;
using mysql_orm::ref;
using mysql_orm::Where;

TEST_CASE("[Where] Where buildquery", "[Where]")
{
  auto table_records = make_table("records",
                                  make_column<&Record::id>("id"),
                                  make_column<&Record::i>("i"),
                                  make_column<&Record::s>("s"));
  auto d = make_database("localhost",
                         3306,
                         "mysql_orm_test",
                         "",
                         "mysql_orm_test_db",
                         table_records);

  CHECK(d.select<Record>()(Where{c<&Record::i>{} == 3}).buildquery() ==
        "SELECT `id`, `i`, `s` FROM `records` WHERE `i`=3");
  SECTION("Where query")
  {
    auto const res =
        d.select<Record>()(Where{c<&Record::i>{} == 4}).build().execute();
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
  auto d = make_database("localhost",
                         3306,
                         "mysql_orm_test",
                         "",
                         "mysql_orm_test_db",
                         table_records);
  d.recreate();
  d.execute(
      "INSERT INTO `records` (`id`, `i`, `s`) VALUES "
      R"((1, 1, "one"),)"
      R"((2, 2, "two"),)"
      R"((3, 4, "four"))");

  SECTION("With value")
  {
    auto const res =
        d.select<Record>()(Where{c<&Record::i>{} == 4}).build().execute();
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
        d.select<Record>()(Where{c<&Record::i>{} == i}).build().execute();
    static_assert(
        std::is_same_v<std::remove_cv_t<decltype(res)>, std::vector<Record>>,
        "Wrong return type");
    REQUIRE(res.size() == 1);
    CHECK(res[0] == Record{3, 4, "four"});
  }

  SECTION("With reference")
  {
    auto i = 3;
    auto const query = d.select<Record>()(Where{c<&Record::i>{} == ref{i}});
    i = 4;
    auto const res = query.build().execute();
    static_assert(
        std::is_same_v<std::remove_cv_t<decltype(res)>, std::vector<Record>>,
        "Wrong return type");
    REQUIRE(res.size() == 1);
    CHECK(res[0] == Record{3, 4, "four"});
  }
}