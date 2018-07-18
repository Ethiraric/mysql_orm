#include <mysql_orm/Limit.hpp>

#include <catch.hpp>

#include <Record.hh>
#include <mysql_orm/Database.hpp>
#include <mysql_orm/GetAll.hpp>

using mysql_orm::c;
using mysql_orm::Limit;
using mysql_orm::make_column;
using mysql_orm::ref;

TEST_CASE("[Limit] Limit buildquery", "[Limit]")
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

  CHECK(d.getAll<Record>()(Limit<2>{}).buildquery() ==
        "SELECT `id`, `i`, `s` FROM `records` LIMIT 2");
}

TEST_CASE("[Limit] Simple Limit", "[Limit]")
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

  SECTION("Templated")
  {
    auto const res =
        d.getAll<RecordWithOptionals>()(Limit<2>{}).build().execute();
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
        d.getAll<RecordWithOptionals>()(Limit<>{i}).build().execute();
    static_assert(std::is_same_v<std::remove_cv_t<decltype(res)>,
                                 std::vector<RecordWithOptionals>>,
                  "Wrong return type");
    REQUIRE(res.size() == 1);
    CHECK(res[0] == RecordWithOptionals{1, 1, "one"});
  }
}
