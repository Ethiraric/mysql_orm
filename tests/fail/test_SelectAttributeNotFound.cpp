#include <mysql_orm/Database.hpp>
#include <mysql_orm/Select.hpp>

#include <Record.hh>

using mysql_orm::make_column;
using mysql_orm::make_database;
using mysql_orm::make_table;

int main()
{
  auto table_records = make_table(
      "records", make_column<&Record::id>("id"), make_column<&Record::s>("s"));
  auto d = make_database("localhost",
                         3306,
                         "mysql_orm_test",
                         "",
                         "mysql_orm_test_db",
                         table_records
                         );

  d.select<&Record::i>();
}
