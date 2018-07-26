#include <mysql_orm/Column.hpp>

#include <catch.hpp>

#include <Record.hh>
#include <mysql_orm/Database.hpp>

using mysql_orm::Autoincrement;
using mysql_orm::Connection;
using mysql_orm::make_column;
using mysql_orm::make_table;
using mysql_orm::make_varchar;
using mysql_orm::PrimaryKey;

TEST_CASE("[Varchar] Column field", "[Varchar][Column]")
{
  auto c = make_varchar<40, &Record::s>("s");
  auto c2 = make_varchar<1, &Record::s>("s");
  CHECK(c.getSchema() == "`s` VARCHAR(40) NOT NULL");
  CHECK(c2.getSchema() == "`s` VARCHAR(1) NOT NULL");
}

TEST_CASE("[Varchar] GetAll", "[Varchar][GetAll]")
{
  auto table_records = make_table("records",
                                  make_column<&Record::id>("id"),
                                  make_column<&Record::i>("i"),
                                  make_varchar<10, &Record::s>("s"));
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

TEST_CASE("[Varchar] Insert", "[Varchar][Insert]")
{
  auto table_records =
      make_table("records",
                 make_column<&Record::id>("id", PrimaryKey{}, Autoincrement{}),
                 make_column<&Record::i>("i"),
                 make_varchar<40, &Record::s>("s"));
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
    CHECK(d.insert(Record{4, 8, "eight"})() == 4);
    auto const res = d.getAll<Record>().build().execute();
    REQUIRE(res.size() == 4);
    CHECK(res[0] == Record{1, 1, "one"});
    CHECK(res[1] == Record{2, 2, "two"});
    CHECK(res[2] == Record{3, 4, "four"});
    CHECK(res[3] == Record{4, 8, "eight"});
  }
}
