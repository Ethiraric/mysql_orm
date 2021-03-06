#ifndef MYSQL_ORM_STATEMENT_HPP_
#define MYSQL_ORM_STATEMENT_HPP_

#include <cstddef>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include <mysql/mysql.h>

#include <mysql_orm/BindArray.hpp>
#include <mysql_orm/Exception.hh>
#include <mysql_orm/QueryType.hpp>

namespace mysql_orm
{

template <typename Query, typename Model>
class Statement
{
public:
  static inline constexpr auto query_type{Query::query_type};

  using SQLQueryType = decltype(std::declval<Query>().buildquery());

  Statement(MYSQL& mysql, Query pquery)
    : mysql_handle{&mysql},
      orm_query{std::move(pquery)},
      sql_query{this->orm_query.buildquery()},
      in_binds{},
      out_binds{},
      stmt{nullptr, &mysql_stmt_close}
  {
    this->stmt.reset(mysql_stmt_init(this->mysql_handle));
    if (!this->stmt)
      throw MySQLException("Failed to create statement: " +
                           std::string{mysql_error(this->mysql_handle)});
    if (mysql_stmt_prepare(
            this->stmt.get(), this->sql_query.c_str(),
            this->sql_query.size()))
      throw MySQLException("Failed to prepare statement: " +
                           std::string{mysql_error(this->mysql_handle)});
    if constexpr (query_type == QueryType::GetAll)
      this->bindOutToQuery();
    if constexpr (query_type != QueryType::Insert)
      this->bindInToQuery();
  }

  Statement(Statement const& b) = delete;
  Statement(Statement&& b) noexcept = default;
  ~Statement() noexcept = default;

  Statement& operator=(Statement const& rhs) = delete;
  Statement& operator=(Statement&& rhs) noexcept = default;

  auto operator()()
  {
    return this->execute();
  }

  void bindInToQuery() noexcept
  {
    this->orm_query.bindInTo(this->in_binds);
  }

  void bindInsert(Model tmp)
  {
    this->temp = std::move(tmp);
    this->orm_query.bindInsert(this->temp, this->in_binds);
  }

  void bindOutToQuery()
  {
    this->orm_query.bindOutTo(this->temp, this->out_binds);
  }

  auto execute()
  {
    if constexpr (query_type == QueryType::GetAll)
    {
      auto ret = std::vector<Model>{};
      this->sql_execute();
      auto errcode = 0;
      while (!(errcode = mysql_stmt_fetch(this->stmt.get())))
      {
        auto copy = this->temp;
        this->orm_query.finalizeBindings(copy, this->out_binds);
        ret.emplace_back(std::move(copy));
      }
      if (errcode != MYSQL_NO_DATA)
        throw MySQLException(mysql_stmt_error(this->stmt.get()));
      return ret;
    }
    else
    {
      this->sql_execute();
      if constexpr (query_type == QueryType::Insert)
        return mysql_stmt_insert_id(this->stmt.get());
    }
  }

private:
  constexpr static size_t getNbOutputSlots() noexcept
  {
    if constexpr (query_type == QueryType::GetAll)
      return Query::getNbOutputSlots();
    else
      return 0;
  }

  void sql_execute()
  {
    this->rebindStdTmReferences();
    auto* mysql_out_binds = const_cast<MYSQL_BIND*>(this->out_binds.data());
    auto* mysql_in_binds = const_cast<MYSQL_BIND*>(this->in_binds.data());
    if ((!this->out_binds.empty() &&
         mysql_stmt_bind_result(this->stmt.get(), mysql_out_binds)) ||
        (!this->in_binds.empty() &&
         mysql_stmt_bind_param(this->stmt.get(), mysql_in_binds)))
      throw MySQLException("Failed to bind statement: " +
                           std::string{mysql_stmt_error(this->stmt.get())});
    if (mysql_stmt_execute(this->stmt.get()))
      throw MySQLException("Failed to execute statement: " +
                           std::string{mysql_stmt_error(this->stmt.get())});
  }

  void rebindStdTmReferences()
  {
    this->orm_query.rebindStdTmReferences(this->in_binds);
  }

  MYSQL* mysql_handle;
  Model temp;
  Query orm_query;
  SQLQueryType sql_query;
  InputBindArray<Query::getNbInputSlots()> in_binds;
  OutputBindArray<Query::getNbOutputSlots()> out_binds;
  std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)> stmt;
};
}

#endif /* !MYSQL_ORM_STATEMENT_HPP_ */
