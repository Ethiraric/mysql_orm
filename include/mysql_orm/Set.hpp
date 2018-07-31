#ifndef MYSQL_ORM_SET_HPP_
#define MYSQL_ORM_SET_HPP_

#include <sstream>

#include <mysql_orm/QueryContinuation.hpp>
#include <mysql_orm/Where.hpp>
#include <mysql_orm/WhereConditionDSL.hpp>

namespace mysql_orm
{
/** Set clause arguments.
 *
 * This class is used as an argument to an `Update`'s `operator()`.
 * The constructor takes as argument the list of assignments to perform.
 * See the `c` and `ref` functions.
 *
 * This class is not the actual query but a class that serves as a tag for the
 * other query classes.
 * The query class is `SetQueryImpl`.
 */
template <typename Assignments>
struct Set
{
  Set(Assignments&& a) : assignments{std::move(a)}
  {
  }

  Assignments assignments;
};

/** A Set query.
 *
 * The class continues an `Update` query.
 * Takes a list of assignments as parameter, which must be an `AssignmentsList`.
 *
 * `buildquery` returns the SQL query as a std::string.
 * `build` returns a `Statement`, which can later be `execute()`d.
 *
 * The `operator()` can be used to continue the query (Where).
 *
 * TODO(ethiraric): Check that all columns from the assignments refer to the
 * model.
 */
template <typename Query, typename Table, typename Assignments>
struct SetQueryImpl
{
public:
  using model_type = typename Query::model_type;
  static inline constexpr auto query_type{Query::query_type};

  SetQueryImpl(MYSQL& mysql, Query q, Table const& t, Assignments&& a) noexcept
    : mysql_handle{&mysql},
      query{std::move(q)},
      table{&t},
      assignments{std::move(a)}
  {
  }
  SetQueryImpl(SetQueryImpl const& b) = default;
  SetQueryImpl(SetQueryImpl&& b) noexcept = default;
  ~SetQueryImpl() noexcept = default;

  SetQueryImpl& operator=(SetQueryImpl const& rhs) = default;
  SetQueryImpl& operator=(SetQueryImpl&& rhs) noexcept = default;

  template <typename Condition>
  constexpr auto operator()(Where<Condition> where) noexcept
  {
    using ContinuationType = QueryContinuation<Query, Table, SetQueryImpl>;
    return WhereQuery<ContinuationType, Table, Condition>{
        *this->mysql_handle,
        static_cast<ContinuationType&>(*this),
        *this->table,
        std::move(where.condition)};
  }

  constexpr auto buildqueryCS() const noexcept
  {
    return this->assignments.appendToQuery(this->query.buildqueryCS() + " SET ",
                                           *this->table);
  }

  constexpr static size_t getNbInputSlots() noexcept
  {
    return Query::getNbInputSlots() + Assignments::getNbInputSlots();
  }

  template <std::size_t NBINDS>
  void bindInTo(InputBindArray<NBINDS>& binds) const noexcept
  {
    this->query.bindInTo(binds);
    this->assignments.bindInTo(binds, this->query.getNbInputSlots());
  }

  template <std::size_t NBINDS>
  void rebindStdTmReferences(InputBindArray<NBINDS>& ins) const noexcept
  {
    this->query.rebindStdTmReferences(ins);
    this->assignments.rebindStdTmReferences(ins, this->query.getNbInputSlots());
  }

protected:
  // May not be nullptr. Can't use std::reference_wrapper since MYSQL is
  // incomplete.
  MYSQL* mysql_handle;
  Query query;
  Table const* table;

private:
  Assignments assignments;
};

template <typename Query, typename Table, typename Condition>
using SetQuery =
    QueryContinuation<Query, Table, SetQueryImpl<Query, Table, Condition>>;
}

#endif /* !MYSQL_ORM_SET_HPP_ */
