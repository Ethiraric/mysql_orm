#ifndef MYSQL_ORM_STATEMENT_HPP_
#define MYSQL_ORM_STATEMENT_HPP_

#include <cstddef>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include <mysql/mysql.h>

#include <mysql_orm/Exception.hh>

namespace mysql_orm
{
template <typename Query, typename Model>
class Statement
{
public:
  Statement(MYSQL& mysql, Query const& query)
    : mysql_handle{&mysql},
      orm_query{query},
      sql_query{query.buildquery()},
      out_binds(query.getNbOutputSlots()),
      out_lengths(query.getNbOutputSlots()),
      out_is_null(query.getNbOutputSlots()),
      out_error(query.getNbOutputSlots()),
      stmt{nullptr, &mysql_stmt_close}
  {
    this->stmt.reset(mysql_stmt_init(this->mysql_handle));
    if (!this->stmt)
      throw MySQLException("Failed to create statement: " +
                           std::string{mysql_error(this->mysql_handle)});
    if (mysql_stmt_prepare(this->stmt.get(), this->sql_query.c_str(), this->sql_query.size()))
      throw MySQLException("Failed to prepare statement: " +
                           std::string{mysql_error(this->mysql_handle)});
    std::memset(this->out_binds.data(),
                0,
                query.getNbOutputSlots() * sizeof(MYSQL_BIND));
    for (auto i = 0u; i < query.getNbOutputSlots(); ++i)
    {
      this->out_binds[i].length = &this->out_lengths[i];
      this->out_binds[i].is_null = &this->out_is_null[i];
      this->out_binds[i].error = &this->out_error[i];
    }
    this->bindOutToQuery(query);
  }

  Statement(Statement const& b) = delete;
  Statement(Statement&& b) noexcept = default;
  ~Statement() noexcept = default;

  Statement& operator=(Statement const& rhs) = delete;
  Statement& operator=(Statement&& rhs) noexcept = default;

  void bindOutToQuery(Query const& query)
  {
    query.bindOutTo(this->temp, this->out_binds);
  }

  auto execute()
  {
    auto ret = std::vector<Model>{};
    this->sql_execute();
    auto errcode = 0;
    while (!(errcode = mysql_stmt_fetch(this->stmt.get())))
    {
      auto copy = this->temp;
      this->orm_query.finalizeBindings(copy,
                                       this->out_binds,
                                       this->out_lengths,
                                       this->out_is_null,
                                       this->out_error);
      ret.emplace_back(std::move(copy));
    }
    if (errcode != MYSQL_NO_DATA)
      throw MySQLException(mysql_stmt_error(this->stmt.get()));
    return ret;
  }

private:
  void sql_execute()
  {
    if (!this->out_binds.empty() &&
        mysql_stmt_bind_result(this->stmt.get(), this->out_binds.data()))
      throw MySQLException("Failed to bind statement: " +
                           std::string{mysql_stmt_error(this->stmt.get())});
    if (mysql_stmt_execute(this->stmt.get()))
      throw MySQLException("Failed to execute statement: " +
                           std::string{mysql_stmt_error(this->stmt.get())});
  }

  MYSQL* mysql_handle;
  Model temp;
  Query orm_query;
  std::string sql_query;
  std::vector<MYSQL_BIND> out_binds;
  std::vector<unsigned long> out_lengths;
  std::vector<my_bool> out_is_null;
  std::vector<my_bool> out_error;
  std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)> stmt;
};
}

#endif /* !MYSQL_ORM_STATEMENT_HPP_ */
