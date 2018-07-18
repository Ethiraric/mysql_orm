#include <mysql_orm/Database.hpp>
#include <mysql_orm/GetAll.hpp>

#include <Record.hh>

using mysql_orm::make_column;
using mysql_orm::make_database;
using mysql_orm::make_table;

int main()
{
  auto table_records = make_table("records",
                                  make_column<&Record::id>("id"),
                                  make_column<&Record::i>("i"),
                                  make_column<&Record::s>("s"));
  auto table_mixed_records = make_table("mixed_records",
                                        make_column<&MixedRecord::id>("id"),
                                        make_column<&MixedRecord::i>("i"),
                                        make_column<&MixedRecord::s>("foo"));
  auto d = make_database("localhost",
                         3306,
                         "mysql_orm_test",
                         "",
                         "mysql_orm_test_db",
                         table_records,
                         table_mixed_records);

  d.getAll<&Record::id, &MixedRecord::id>();
}
