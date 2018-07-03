#ifndef TESTS_RECORD_HH_
#define TESTS_RECORD_HH_

#include <chrono>
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

struct RecordWithTime
{
  mysql_orm::id_t id;
  std::tm time;

  bool operator==(RecordWithTime const& b) const noexcept
  {
    return this->id == b.id && this->time.tm_sec == b.time.tm_sec &&
           this->time.tm_min == b.time.tm_min &&
           this->time.tm_hour == b.time.tm_hour &&
           this->time.tm_mday == b.time.tm_mday &&
           this->time.tm_mon == b.time.tm_mon &&
           this->time.tm_year == b.time.tm_year;
  }
};

inline std::ostream& operator<<(std::ostream& out, RecordWithTime const& record)
{ 
  // clang-format off
  out << "RecordWithTime{" << record.id << ','
    << record.time.tm_year + 1900 << '-'
    << record.time.tm_mon + 1 << '-'
    << record.time.tm_mday << ' '
    << record.time.tm_hour << ':'
    << record.time.tm_min << ':'
    << record.time.tm_sec << '}';
  // clang-format on
  return out;
}

struct LargeRecord
{
  mysql_orm::id_t id;
  int a, b, c, d, e, f, g;

  bool operator==(LargeRecord const& o) const noexcept
  {
    return this->id == o.id && this->a == o.a && this->b == o.b &&
           this->c == o.c && this->d == o.d && this->e == o.e &&
           this->f == o.f && this->g == o.g;
  }
};

inline std::ostream& operator<<(std::ostream& out, LargeRecord const& record)
{
  out << "LargeRecord{" << record.id << ',' << record.a << ',' << record.b
      << ',' << record.c << ',' << record.d << ',' << record.e << ','
      << record.f << ',' << record.g << '}';
  return out;
}

inline std::tm makeTm(int year, int mon, int mday, int hour, int min, int sec)
{
  auto tm = std::tm{};

  tm.tm_year = year - 1900;
  tm.tm_mon = mon - 1;
  tm.tm_mday = mday;
  tm.tm_hour = hour;
  tm.tm_min = min;
  tm.tm_sec = sec;
  return tm;
}

#endif /* !TESTS_RECORD_HH_ */
