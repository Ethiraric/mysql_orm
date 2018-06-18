#include <mysql_orm/Select.hpp>

#include <catch.hpp>

#include <Record.hh>
#include <mysql_orm/Database.hpp>
#include <mysql_orm/Where.hpp>

using mysql_orm::c;
using mysql_orm::Limit;
using mysql_orm::make_column;
using mysql_orm::make_database;
using mysql_orm::make_table;
using mysql_orm::MySQLException;
using mysql_orm::ref;
using mysql_orm::Where;

TEST_CASE("[Select] Select statement", "[Select]")
{
  auto table_mixed_records = make_table("mixed_records",
                                        make_column<&MixedRecord::id>("id"),
                                        make_column<&MixedRecord::i>("i"),
                                        make_column<&MixedRecord::s>("foo"));
  auto table_records = make_table("records",
                                  make_column<&Record::id>("id"),
                                  make_column<&Record::i>("i"),
                                  make_column<&Record::s>("s"));

  auto d = make_database("localhost",
                         3306,
                         "mysql_orm_test",
                         "",
                         "mysql_orm_test_db",
                         table_mixed_records,
                         table_records);

  SECTION("Select")
  {
    CHECK(d.select<MixedRecord>().buildquery() ==
          "SELECT `id`, `i`, `foo` FROM `mixed_records`");
  }
}

TEST_CASE("[Select] Select", "[Select]")
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

  SECTION("One row")
  {
    d.execute("DELETE FROM `records` WHERE `id`>1");
    auto const res = d.select<Record>().build().execute();
    static_assert(
        std::is_same_v<std::remove_cv_t<decltype(res)>, std::vector<Record>>,
        "Wrong return type");
    REQUIRE(res.size() == 1);
    CHECK(res[0] == Record{1, 1, "one"});
  }

  SECTION("All rows")
  {
    auto const res = d.select<Record>().build().execute();
    static_assert(
        std::is_same_v<std::remove_cv_t<decltype(res)>, std::vector<Record>>,
        "Wrong return type");
    REQUIRE(res.size() == 3);
    CHECK(res[0] == Record{1, 1, "one"});
    CHECK(res[1] == Record{2, 2, "two"});
    CHECK(res[2] == Record{3, 4, "four"});
  }

  SECTION("Where query")
  {
    auto const res =
        d.select<Record>()(mysql_orm::Where{mysql_orm::c<&Record::i>{} == 4})
            .build()
            .execute();
    static_assert(
        std::is_same_v<std::remove_cv_t<decltype(res)>, std::vector<Record>>,
        "Wrong return type");
    REQUIRE(res.size() == 1);
    CHECK(res[0] == Record{3, 4, "four"});
  }
}

TEST_CASE("[Select] Select with optionals", "[Select]")
{
  auto table_records = make_table("optional_records",
                                  make_column<&RecordWithOptionals::id>("id"),
                                  make_column<&RecordWithOptionals::i>("i"),
                                  make_column<&RecordWithOptionals::s>("s"));
  auto d = make_database("localhost",
                         3306,
                         "mysql_orm_test",
                         "",
                         "mysql_orm_test_db",
                         table_records);
  d.recreate();
  d.execute(
      "INSERT INTO `optional_records` (`id`, `i`, `s`) VALUES "
      R"((1, 1, "one"),)"
      R"((2, 2, "two"),)"
      R"((3, 4, "four"))");

  SECTION("One row")
  {
    d.execute("DELETE FROM `optional_records` WHERE `id`>1");
    auto const res = d.select<RecordWithOptionals>().build().execute();
    static_assert(std::is_same_v<std::remove_cv_t<decltype(res)>,
                                 std::vector<RecordWithOptionals>>,
                  "Wrong return type");
    REQUIRE(res.size() == 1);
    CHECK(res[0] == RecordWithOptionals{1, 1, "one"});
  }

  SECTION("All rows")
  {
    auto const res = d.select<RecordWithOptionals>().build().execute();
    static_assert(std::is_same_v<std::remove_cv_t<decltype(res)>,
                                 std::vector<RecordWithOptionals>>,
                  "Wrong return type");
    REQUIRE(res.size() == 3);
    CHECK(res[0] == RecordWithOptionals{1, 1, "one"});
    CHECK(res[1] == RecordWithOptionals{2, 2, "two"});
    CHECK(res[2] == RecordWithOptionals{3, 4, "four"});
  }

  SECTION("Where query")
  {
    SECTION("With Value")
    {
      auto const res =
          d.select<RecordWithOptionals>()(
               mysql_orm::Where{mysql_orm::c<&RecordWithOptionals::i>{} == 4})
              .build()
              .execute();
      static_assert(std::is_same_v<std::remove_cv_t<decltype(res)>,
                                   std::vector<RecordWithOptionals>>,
                    "Wrong return type");
      REQUIRE(res.size() == 1);
      CHECK(res[0] == RecordWithOptionals{3, 4, "four"});
    }

    SECTION("With variable")
    {
      auto const i = 4;
      auto const res =
          d.select<RecordWithOptionals>()(
               mysql_orm::Where{mysql_orm::c<&RecordWithOptionals::i>{} == i})
              .build()
              .execute();
      static_assert(std::is_same_v<std::remove_cv_t<decltype(res)>,
                                   std::vector<RecordWithOptionals>>,
                    "Wrong return type");
      REQUIRE(res.size() == 1);
      CHECK(res[0] == RecordWithOptionals{3, 4, "four"});
    }

    SECTION("With reference")
    {
      auto i = 3;
      auto const query = d.select<RecordWithOptionals>()(
          mysql_orm::Where{mysql_orm::c<&RecordWithOptionals::i>{} == ref{i}});
      i = 4;
      auto const res = query.build().execute();
      static_assert(std::is_same_v<std::remove_cv_t<decltype(res)>,
                                   std::vector<RecordWithOptionals>>,
                    "Wrong return type");
      REQUIRE(res.size() == 1);
      CHECK(res[0] == RecordWithOptionals{3, 4, "four"});
    }
  }

  SECTION("Limit query")
  {
    SECTION("Templated")
    {
      auto const res =
          d.select<RecordWithOptionals>()(Limit<2>{}).build().execute();
      static_assert(std::is_same_v<std::remove_cv_t<decltype(res)>,
                                   std::vector<RecordWithOptionals>>,
                    "Wrong return type");
      REQUIRE(res.size() == 2);
      CHECK(res[0] == RecordWithOptionals{1, 1, "one"});
      CHECK(res[1] == RecordWithOptionals{2, 2, "two"});
    }

    SECTION("With variable")
    {
      auto const i = size_t{1};
      auto const res =
          d.select<RecordWithOptionals>()(Limit<>{i}).build().execute();
      static_assert(std::is_same_v<std::remove_cv_t<decltype(res)>,
                                   std::vector<RecordWithOptionals>>,
                    "Wrong return type");
      REQUIRE(res.size() == 1);
      CHECK(res[0] == RecordWithOptionals{1, 1, "one"});
    }
  }
}
