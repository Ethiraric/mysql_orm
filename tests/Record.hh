#ifndef TESTS_RECORD_HH_
#define TESTS_RECORD_HH_

#include <optional>
#include <string>

#include <mysql_orm/Column.hpp>

struct Record
{
  mysql_orm::id_t id;
  int i;
  std::string s;
};

struct RecordWithOptionals
{
  std::optional<mysql_orm::id_t> id;
  std::optional<int> i;
  std::optional<std::string> s;
};

struct MixedRecord
{
  mysql_orm::id_t id;
  int i;
  std::optional<std::string> s;
};

#endif /* !TESTS_RECORD_HH_ */
