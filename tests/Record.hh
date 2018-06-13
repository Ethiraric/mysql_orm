#ifndef TESTS_RECORD_HH_
#define TESTS_RECORD_HH_

#include <optional>
#include <ostream>
#include <string>

#include <mysql_orm/Column.hpp>

struct Record
{
  mysql_orm::id_t id;
  int i;
  std::string s;

  bool operator==(Record const& b) const noexcept
  {
    return this->id == b.id && this->i == b.i && this->s == b.s;
  }
};

inline std::ostream& operator<<(std::ostream& out, Record const& record)
{
  out << "Record{" << record.id << ',' << record.i << ",`" << record.s << "`}";
  return out;
}

struct RecordWithOptionals
{
  std::optional<mysql_orm::id_t> id;
  std::optional<int> i;
  std::optional<std::string> s;
  bool operator==(RecordWithOptionals const& b) const noexcept
  {
    return this->id == b.id && this->i == b.i && this->s == b.s;
  }
};

struct MixedRecord
{
  mysql_orm::id_t id;
  int i;
  std::optional<std::string> s;

  bool operator==(MixedRecord const& b) const noexcept
  {
    return this->id == b.id && this->i == b.i && this->s == b.s;
  }
};

#endif /* !TESTS_RECORD_HH_ */
