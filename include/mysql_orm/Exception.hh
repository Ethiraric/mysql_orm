#ifndef MYSQL_ORM_EXCEPTION_HH_
#define MYSQL_ORM_EXCEPTION_HH_

#include <stdexcept>
#include <string>

#include <mysql/mysqld_error.h>

namespace mysql_orm
{
class Exception : public std::exception
{
public:
  explicit Exception(std::string what = "") noexcept : _what{std::move(what)}
  {
  }

  virtual ~Exception() noexcept
  {
  }

  char const* what() const noexcept override
  {
    return this->_what.c_str();
  }

private:
  std::string _what;
};

class MySQLException : public Exception
{
public:
  explicit MySQLException(std::string what = "") noexcept
    : Exception(std::move(what))
  {
  }
};

/** Used to indicate an error in a MySQL query.
 */
class MySQLQueryException : public MySQLException
{
public:
  MySQLQueryException(int errcode, char const* what) noexcept
    : MySQLException(what), _errcode{errcode}
  {
  }

  int errcode() const noexcept
  {
    return this->_errcode;
  }

  bool isTableDoesNotExist() const noexcept
  {
    return this->errcode() == ER_NO_SUCH_TABLE;
  }

protected:
  int _errcode;
};
}

#endif /* !MYSQL_ORM_EXCEPTION_HH_ */
