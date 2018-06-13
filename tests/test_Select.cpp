#include <mysql_orm/Database.hpp>

#include <catch.hpp>

#include <Record.hh>

using mysql_orm::make_column;
using mysql_orm::make_database;
using mysql_orm::make_table;
using mysql_orm::MySQLException;

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

TEST_CASE("[Select] Selecting one record", "[Select]")
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
  d.execute(R"(INSERT INTO `records` (`id`, `i`, `s`) VALUES (1, 1, "one"))");

  SECTION("Select")
  {
    auto const res = d.select<Record>().build().execute();
    static_assert(
        std::is_same_v<std::remove_cv_t<decltype(res)>, std::vector<Record>>,
        "Wrong return type");
    REQUIRE(res.size() == 1);
    CHECK(res[0] == Record{1, 1, "one"});
  }
}
