#ifndef MYSQL_ORM_DATABASE_HPP_
#define MYSQL_ORM_DATABASE_HPP_

#include <memory>
#include <tuple>

#include <mysql/mysql.h>

#include <mysql_orm/Exception.hh>
#include <mysql_orm/Table.hpp>

namespace mysql_orm
{
template <typename... Tables>
class Database
{
public:
  Database(std::string const& host,
           unsigned short port,
           std::string const& username,
           std::string const& password,
           std::string const& database,
           Tables&&... tabls)
    : handle{mysql_init(nullptr), &mysql_close},
      tables{std::forward_as_tuple(tabls...)}
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

  Database(Database const& b) noexcept = delete;
  Database(Database&& b) noexcept = default;
  ~Database() noexcept = default;

  Database& operator=(Database const& rhs) noexcept = delete;
  Database& operator=(Database&& rhs) noexcept = default;

  void execute(std::string const& query)
  {
    if (mysql_real_query(this->handle.get(), query.c_str(), query.size()))
      throw MySQLQueryException(mysql_errno(this->handle.get()),
                                mysql_error(this->handle.get()));
  }

private:
  std::unique_ptr<MYSQL, decltype(&mysql_close)> handle;
  std::tuple<Tables...> tables;
};

template <typename... Tables>
auto make_database(std::string const& host,
                   unsigned short port,
                   std::string const& username,
                   std::string const& password,
                   std::string const& database,
                   Tables&&... tables)
{
  return Database<Tables...>{host,
                             port,
                             username,
                             password,
                             database,
                             std::forward<Tables>(tables)...};
}
}

#endif /* !MYSQL_ORM_DATABASE_HPP_ */
