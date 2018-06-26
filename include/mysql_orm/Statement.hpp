#ifndef MYSQL_ORM_STATEMENT_HPP_
#define MYSQL_ORM_STATEMENT_HPP_

#include <cstddef>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include <mysql/mysql.h>

#include <mysql_orm/Exception.hh>
#include <mysql_orm/QueryType.hpp>

namespace mysql_orm
{
template <typename Query, typename Model>
class Statement
{
public:
  static inline constexpr auto query_type{Query::query_type};

  Statement(MYSQL& mysql, Query pquery)
    : mysql_handle{&mysql},
      orm_query{std::move(pquery)},
      sql_query{this->orm_query.buildquery()},
      in_binds{this->orm_query.getNbInputSlots()},
      out_binds(this->getNbOutputSlots()),
      out_lengths(this->getNbOutputSlots()),
      out_is_null(this->getNbOutputSlots()),
      out_error(this->getNbOutputSlots()),
      stmt{nullptr, &mysql_stmt_close}
  {
    this->stmt.reset(mysql_stmt_init(this->mysql_handle));
    if (!this->stmt)
      throw MySQLException("Failed to create statement: " +
                           std::string{mysql_error(this->mysql_handle)});
    if (mysql_stmt_prepare(
            this->stmt.get(), this->sql_query.c_str(), this->sql_query.size()))
      throw MySQLException("Failed to prepare statement: " +
                           std::string{mysql_error(this->mysql_handle)});
    std::memset(
        this->out_binds.data(), 0, this->out_binds.size() * sizeof(MYSQL_BIND));
    if constexpr (query_type == QueryType::Select)
    {
      std::memset(
          this->in_binds.data(), 0, this->in_binds.size() * sizeof(MYSQL_BIND));
      for (auto i = 0u; i < this->orm_query.getNbOutputSlots(); ++i)
      {
        this->out_binds[i].length = &this->out_lengths[i];
        this->out_binds[i].is_null = &this->out_is_null[i];
        this->out_binds[i].error = &this->out_error[i];
      }
      this->bindOutToQuery();
    }
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
    this->orm_query.bindInTo(&this->in_binds[0]);
  }

  void bindInsert(Model tmp)
  {
    this->temp = std::move(tmp);
    this->orm_query.bindInsert(this->temp, &this->in_binds[0]);
  }

  void bindOutToQuery()
  {
    this->orm_query.bindOutTo(this->temp, this->out_binds);
  }

  auto execute()
  {
    if constexpr (query_type == QueryType::Select)
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
    else
      this->sql_execute();
  }

private:
  size_t getNbOutputSlots() const noexcept
  {
    if constexpr (query_type == QueryType::Select)
      return this->orm_query.getNbOutputSlots();
    else
      return 0;
  }

  void sql_execute()
  {
    if ((!this->out_binds.empty() &&
         mysql_stmt_bind_result(this->stmt.get(), this->out_binds.data())) ||
        (!this->in_binds.empty() &&
         mysql_stmt_bind_param(this->stmt.get(), this->in_binds.data())))
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
  std::vector<MYSQL_BIND> in_binds;
  std::vector<MYSQL_BIND> out_binds;
  std::vector<unsigned long> out_lengths;
  std::vector<my_bool> out_is_null;
  std::vector<my_bool> out_error;
  std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)> stmt;
};
}

#endif /* !MYSQL_ORM_STATEMENT_HPP_ */
