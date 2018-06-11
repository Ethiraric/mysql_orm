#ifndef MYSQL_ORM_STATEMENT_HPP_
#define MYSQL_ORM_STATEMENT_HPP_

#include <cstddef>
#include <string>
#include <vector>

#include <mysql/mysql.h>

template <typename Model>
class Statement
{
public:
  Statement(MYSQL& mysql, std::string pquery, size_t nout_binds) noexcept
    : mysql_handle{&mysql}, query{std::move(pquery)}
  {
    this->out_binds.reserve(nout_binds);
  }
  Statement(Statement const& b) = default;
  Statement(Statement&& b) noexcept = default;
  ~Statement() noexcept = default;

  Statement& operator=(Statement const& rhs) = default;
  Statement& operator=(Statement&& rhs) noexcept = default;

private:
  MYSQL* mysql_handle;
  Model temp;
  std::string query;
  std::vector<MYSQL_BIND> out_binds;
};

#endif /* !MYSQL_ORM_STATEMENT_HPP_ */
