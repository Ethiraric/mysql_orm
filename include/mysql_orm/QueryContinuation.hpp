#ifndef MYSQL_ORM_QUERYCONTINUATION_HPP_
#define MYSQL_ORM_QUERYCONTINUATION_HPP_

#include <functional>

#include <mysql/mysql.h>

#include <mysql_orm/Statement.hpp>

namespace mysql_orm
{
template <typename Query, typename Table, typename Continuation>
class QueryContinuation : public Continuation
{
public:
  using model_type = typename Query::model_type;

  template <typename... ContinuationArgs>
  QueryContinuation(MYSQL& mysql,
                    Query q,
                    Table const& t,
                    ContinuationArgs... args) noexcept
    : Continuation{
          mysql, std::move(q), t, std::forward<ContinuationArgs>(args)...}
  {
  }
  QueryContinuation(QueryContinuation const& b) noexcept = default;
  QueryContinuation(QueryContinuation&& b) noexcept = default;
  ~QueryContinuation() noexcept = default;

  QueryContinuation& operator=(QueryContinuation const& rhs) noexcept = default;
  QueryContinuation& operator=(QueryContinuation&& rhs) noexcept = default;

  std::string buildquery() const
  {
    return this->buildqueryss().str();
  }

  size_t getNbOutputSlots() const noexcept
  {
    return this->query.getNbOutputSlots();
  }

  void bindOutTo(model_type& model, std::vector<MYSQL_BIND>& out_binds) const
  {
    this->query.bindOutTo(model, out_binds);
  }

  void finalizeBindings(model_type& model,
                        std::vector<MYSQL_BIND>& binds,
                        std::vector<unsigned long>& lengths,
                        std::vector<my_bool>& are_null,
                        std::vector<my_bool>& errors)
  {
    this->query.finalizeBindings(model, binds, lengths, are_null, errors);
  }

  Statement<QueryContinuation, model_type> build() const
  {
    return Statement<QueryContinuation, model_type>{*this->mysql_handle, *this};
  }
};

template <typename Query, typename Table, typename Continuation>
QueryContinuation(Query, Table, Continuation)
    ->QueryContinuation<Query, Table, Continuation>;
}

#endif /* !MYSQL_ORM_QUERYCONTINUATION_HPP_ */
