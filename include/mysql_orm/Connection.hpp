#ifndef MYSQL_ORM_CONNECTION_HPP_
#define MYSQL_ORM_CONNECTION_HPP_

#include <memory>
#include <string>

#include <mysql/mysql.h>

#include <mysql_orm/Exception.hh>

namespace mysql_orm
{
template <typename...>
class Database;

class Connection
{
public:
  Connection(std::string const& host,
             unsigned short port,
             std::string const& username,
             std::string const& password,
             std::string const& database)
    : handle{mysql_init(nullptr), &mysql_close}
  {
    if (!mysql_real_connect(this->handle.get(),
                            host.c_str(),
                            username.c_str(),
                            password.c_str(),
                            database.c_str(),
                            port,
                            nullptr,
                            CLIENT_MULTI_STATEMENTS))
      throw MySQLException("Failed to connect to database");

    auto reconnect = my_bool{1};
    if (mysql_options(this->handle.get(), MYSQL_OPT_RECONNECT, &reconnect))
      throw MySQLException("Failed to set MySQL autoreconnect");
  }

  Connection(Connection const& b) noexcept = delete;
  Connection(Connection&& b) noexcept = default;
  ~Connection() noexcept = default;

  Connection& operator=(Connection const& rhs) noexcept = delete;
  Connection& operator=(Connection&& rhs) noexcept = default;

private:
  template <typename...>
  friend class Database;

  MYSQL* getHandle() noexcept
  {
    return this->handle.get();
  }

  std::unique_ptr<MYSQL, decltype(&mysql_close)> handle;
};
}

#endif /* !MYSQL_ORM_CONNECTION_HPP_ */
