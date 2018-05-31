#include <mysql_orm/Table.hpp>

#include <catch.hpp>

#include <string>

namespace
{
struct Record
{
  mysql_orm::id_t id;
  int i;
  std::optional<std::string> s;
};
}

using mysql_orm::make_column;
using mysql_orm::make_table;

TEST_CASE("Create statement", "[Table]")
{
  auto t = make_table("record",
                      make_column<&Record::id>("id"),
                      make_column<&Record::i>("i"),
                      make_column<&Record::s>("foo"));
  CHECK(t.getSchema() ==
        "CREATE TABLE `record` (\n"
        "  `id` INTEGER UNSIGNED NOT NULL,\n"
        "  `i` INTEGER NOT NULL,\n"
        "  `foo` TEXT\n"
        ")");
}
