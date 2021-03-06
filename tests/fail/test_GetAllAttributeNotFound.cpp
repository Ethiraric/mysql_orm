#include <mysql_orm/Database.hpp>
#include <mysql_orm/GetAll.hpp>

#include <Record.hh>

using mysql_orm::Connection;
using mysql_orm::make_column;
using mysql_orm::make_database;
using mysql_orm::make_table;

int main()
{
  auto table_records = make_table(
      "records", make_column<&Record::id>("id"), make_column<&Record::s>("s"));
  auto connection =
      Connection{"localhost", 3306, "mysql_orm_test", "", "mysql_orm_test_db"};
  auto d = make_database(connection, table_records);

  d.getAll<&Record::i>();
}
