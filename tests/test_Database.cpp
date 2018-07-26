#include <mysql_orm/Database.hpp>

#include <catch.hpp>

#include <Record.hh>

using mysql_orm::Connection;
using mysql_orm::make_column;
using mysql_orm::make_database;
using mysql_orm::make_table;
using mysql_orm::MySQLException;

TEST_CASE("Create database", "[Database]")
{
  auto table = make_table("record",
                          make_column<&MixedRecord::id>("id"),
                          make_column<&MixedRecord::i>("i"),
                          make_column<&MixedRecord::s>("foo"));
  auto connection =
      Connection{"localhost", 3306, "mysql_orm_test", "", "mysql_orm_test_db"};
  auto d = make_database(connection, table);
}
