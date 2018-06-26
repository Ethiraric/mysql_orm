#include <mysql_orm/Insert.hpp>

#include <catch.hpp>

#include <Record.hh>
#include <mysql_orm/Database.hpp>

using mysql_orm::c;
using mysql_orm::Limit;
using mysql_orm::make_column;
using mysql_orm::make_database;
using mysql_orm::make_table;
using mysql_orm::MySQLException;
using mysql_orm::ref;
using mysql_orm::Where;

TEST_CASE("[Insert] Insert buildquery", "[Insert]")
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

  auto mr = MixedRecord{};
  auto r = Record{};
  CHECK(d.insert(mr).buildquery() ==
        "INSERT INTO `mixed_records` (`id`, `i`, `foo`) VALUES (?, ?, ?)");
  CHECK(d.insert<&Record::i, &Record::s>(r).buildquery() ==
        "INSERT INTO `records` (`i`, `s`) VALUES (?, ?)");
}

TEST_CASE("[Insert] Insert", "[Insert]")
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
    d.insert(Record{4, 8, "eight"})();
    auto const res = d.select<Record>().build().execute();
    static_assert(
        std::is_same_v<std::remove_cv_t<decltype(res)>, std::vector<Record>>,
        "Wrong return type");
    REQUIRE(res.size() == 4);
    CHECK(res[0] == Record{1, 1, "one"});
    CHECK(res[1] == Record{2, 2, "two"});
    CHECK(res[2] == Record{3, 4, "four"});
    CHECK(res[3] == Record{4, 8, "eight"});
  }
}
