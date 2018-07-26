#include <mysql_orm/Update.hpp>

#include <catch.hpp>

#include <Record.hh>
#include <mysql_orm/Database.hpp>
#include <mysql_orm/Set.hpp>
#include <mysql_orm/Where.hpp>

using mysql_orm::Autoincrement;
using mysql_orm::c;
using mysql_orm::Connection;
using mysql_orm::make_column;
using mysql_orm::make_database;
using mysql_orm::make_table;
using mysql_orm::MySQLException;
using mysql_orm::PrimaryKey;
using mysql_orm::ref;
using mysql_orm::Set;
using mysql_orm::Update;
using mysql_orm::Where;

TEST_CASE("[Update] Update buildquery", "[Update]")
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

  CHECK(d.update<Record>()(Set{c<&Record::i>{} = 3}).buildquery() ==
        "UPDATE `records` SET `i`=?");
}

TEST_CASE("[Update] Update", "[Update]")
{
  auto table_records = make_table("records",
                                  make_column<&Record::id>("id"),
                                  make_column<&Record::i>("i"),
                                  make_column<&Record::s>("s"));
  auto table_large_records = make_table(
      "large_records",
      make_column<&LargeRecord::id>("id", Autoincrement{}, PrimaryKey{}),
      make_column<&LargeRecord::a>("a"),
      make_column<&LargeRecord::b>("b"),
      make_column<&LargeRecord::c>("c"),
      make_column<&LargeRecord::d>("d"),
      make_column<&LargeRecord::e>("e"),
      make_column<&LargeRecord::f>("f"),
      make_column<&LargeRecord::g>("g"));
  auto connection =
      Connection{"localhost", 3306, "mysql_orm_test", "", "mysql_orm_test_db"};
  auto d = make_database(connection, table_records, table_large_records);

  d.recreate();
  d.execute(
      "INSERT INTO `records` (`id`, `i`, `s`) VALUES "
      R"((1, 1, "one"),)"
      R"((2, 2, "two"),)"
      R"((3, 4, "four"))");
  d.execute(
      "INSERT INTO `large_records` (`id`, `a`, `b`, `c`, `d`, `e`, `f`, `g`)"
      " VALUES (1, 0, 1, 2, 3, 4, 5, 6)");

  SECTION("All rows")
  {
    d.update<Record>()(Set{c<&Record::i>{} = 3}).build().execute();
    auto const res = d.getAll<Record>().build().execute();
    static_assert(
        std::is_same_v<std::remove_cv_t<decltype(res)>, std::vector<Record>>,
        "Wrong return type");
    REQUIRE(res.size() == 3);
    CHECK(res[0] == Record{1, 3, "one"});
    CHECK(res[1] == Record{2, 3, "two"});
    CHECK(res[2] == Record{3, 3, "four"});
  }

  SECTION("With Where")
  {
    d.update<Record>()(Set{c<&Record::i>{} = 3})(Where{c<&Record::id>() == 2})
        .build()
        .execute();
    auto const res = d.getAll<Record>().build().execute();
    static_assert(
        std::is_same_v<std::remove_cv_t<decltype(res)>, std::vector<Record>>,
        "Wrong return type");
    REQUIRE(res.size() == 3);
    CHECK(res[0] == Record{1, 1, "one"});
    CHECK(res[1] == Record{2, 3, "two"});
    CHECK(res[2] == Record{3, 4, "four"});
  }

  SECTION("Set two columns")
  {
    d.update<Record>()(Set{(c<&Record::i>{} = 3, c<&Record::id>() = 4)})(
         Where{c<&Record::id>() == 2})
        .build()
        .execute();
    auto const res = d.getAll<Record>().build().execute();
    static_assert(
        std::is_same_v<std::remove_cv_t<decltype(res)>, std::vector<Record>>,
        "Wrong return type");
    REQUIRE(res.size() == 3);
    CHECK(res[0] == Record{1, 1, "one"});
    CHECK(res[1] == Record{4, 3, "two"});
    CHECK(res[2] == Record{3, 4, "four"});
  }

  SECTION("Set a lot of columns")
  {
    d.update<LargeRecord>()(Set{(c<&LargeRecord::a>{} = 1,
                                 c<&LargeRecord::b>{} = 2,
                                 c<&LargeRecord::c>{} = 3,
                                 c<&LargeRecord::d>{} = 4,
                                 c<&LargeRecord::e>{} = 5,
                                 c<&LargeRecord::f>{} = 6,
                                 c<&LargeRecord::g>{} = 7)})();
    auto const res = d.getAll<LargeRecord>()();
    static_assert(std::is_same_v<std::remove_cv_t<decltype(res)>,
                                 std::vector<LargeRecord>>,
                  "Wrong return type");
    REQUIRE(res.size() == 1);
    CHECK(res[0] == LargeRecord{1, 1, 2, 3, 4, 5, 6, 7});
  }
}
