#ifndef MYSQL_ORM_DATABASE_HPP_
#define MYSQL_ORM_DATABASE_HPP_

#include <memory>
#include <tuple>

#include <mysql/mysql.h>

#include <mysql_orm/Connection.hpp>
#include <mysql_orm/Delete.hpp>
#include <mysql_orm/Exception.hh>
#include <mysql_orm/Table.hpp>
#include <mysql_orm/Update.hpp>
#include <mysql_orm/meta/AllSame.hpp>
#include <mysql_orm/meta/AttributePtrDissector.hpp>
#include <mysql_orm/meta/FindMapped.hpp>
#include <mysql_orm/meta/TableModelGetter.hpp>

namespace mysql_orm
{
template <typename... Tables>
class Database
{
public:
  Database(Connection& hdl, Tables&&... tabls)
    : handle{hdl},
      tables{std::forward_as_tuple(tabls...)}
  {
  }

  Database(Database const& b) noexcept = delete;
  Database(Database&& b) noexcept = default;
  ~Database() noexcept = default;

  Database& operator=(Database const& rhs) noexcept = delete;
  Database& operator=(Database&& rhs) noexcept = default;

  void execute(std::string const& query)
  {
    if (mysql_real_query(this->getMYSQLHandle(), query.c_str(), query.size()))
      throw MySQLQueryException(mysql_errno(this->getMYSQLHandle()),
                                mysql_error(this->getMYSQLHandle()));
  }

  template <typename Model>
  auto getAll()
  {
    return this->getTable<Model>().getAll(*this->getMYSQLHandle());
  }

  template <auto Attr, auto... Attrs>
  auto getAll()
  {
    this->checkAttributes<Attr, Attrs...>();
    using Model_t = meta::AttributeModelGetter_t<decltype(Attr)>;
    return this->getTable<Model_t>().template getAll<Attr, Attrs...>(
        *this->getMYSQLHandle());
  }

  template <typename Model>
  auto insert(Model const& model)
  {
    return this->getTable<Model>().insert(*this->getMYSQLHandle(), &model);
  }

  template <auto Attr,
            auto... Attrs,
            typename Model = meta::AttributeModelGetter_t<decltype(Attr)>>
  auto insert(Model const& model)
  {
    this->checkAttributes<Attr, Attrs...>();
    using Model_t = meta::AttributeModelGetter_t<decltype(Attr)>;
    return this->getTable<Model_t>().template insert<Attr, Attrs...>(
        *this->getMYSQLHandle(), &model);
  }

  template <auto Attr,
            auto... Attrs,
            typename Model = meta::AttributeModelGetter_t<decltype(Attr)>>
  auto insertAllBut(Model const& model)
  {
    this->checkAttributes<Attr, Attrs...>();
    using Model_t = meta::AttributeModelGetter_t<decltype(Attr)>;
    return this->getTable<Model_t>().template insertAllBut<Attr, Attrs...>(
        *this->getMYSQLHandle(), &model);
  }

  template <typename Model>
  auto update()
  {
    using Table_t = typename meta::
        FindMapped<meta::TableModelGetter, Model, Tables...>::type;
    static_assert(!std::is_same_v<Table_t, void>,
                  "Failed to find table for model");
    return Update<std::remove_reference_t<Table_t>>{
        *this->getMYSQLHandle(), std::get<Table_t>(this->tables)};
  }

  template <typename Model>
  auto delete_()
  {
    using Table_t = typename meta::
        FindMapped<meta::TableModelGetter, Model, Tables...>::type;
    static_assert(!std::is_same_v<Table_t, void>,
                  "Failed to find table for model");
    return Delete<std::remove_reference_t<Table_t>>{
        *this->getMYSQLHandle(), std::get<Table_t>(this->tables)};
  }

  void recreate()
  {
    for_each_tuple(this->tables, [&](auto const& table) {
      this->execute("DROP TABLE IF EXISTS `" + table.getName() + '`');
      this->execute(table.getSchema());
    });
  }

  template <typename Model>
  void recreate()
  {
    auto& table = this->getTable<Model>();
    this->execute("DROP TABLE IF EXISTS `" + table.getName() + '`');
    this->execute(table.getSchema());
  }

  void create()
  {
    for_each_tuple(this->tables, [&](auto const& table) {
      this->execute(table.getSchema());
    });
  }

  template <typename Model>
  void create()
  {
    auto& table = this->getTable<Model>();
    this->execute(table.getSchema());
  }

  void drop()
  {
    // TODO(ethiraric): Merge these into one single query.
    for_each_tuple(this->tables, [&](auto const& table) {
      this->execute("DROP TABLE IF EXISTS `" + table.getName() + '`');
    });
  }

  template <typename Model>
  void drop()
  {
    auto& table = this->getTable<Model>();
    this->execute("DROP TABLE IF EXISTS `" + table.getName() + '`');
  }

private:
  MYSQL* getMYSQLHandle() noexcept
  {
    return this->handle.get().getHandle();
  }

  template <typename Model>
  auto& getTable()
  {
    using Table_t = typename meta::
        FindMapped<meta::TableModelGetter, Model, Tables...>::type;
    static_assert(!std::is_same_v<Table_t, void>,
                  "Failed to find table for model");
    return std::get<Table_t>(this->tables);
  }

  template <auto Attr, auto... Attrs>
  void checkAttributes() const noexcept
  {
    static_assert(
        meta::AllSame_v<meta::AttributeModelGetter_t<decltype(Attr)>,
                        meta::AttributeModelGetter_t<decltype(Attrs)>...>,
        "Attributes do not refer to the same model");
    using Model_t = meta::AttributeModelGetter_t<decltype(Attr)>;
    using Table_t = typename meta::
        FindMapped<meta::TableModelGetter, Model_t, Tables...>::type;
    static_assert(!std::is_same_v<Table_t, void>,
                  "Failed to find table for model");
  }

  std::reference_wrapper<Connection> handle;
  std::tuple<Tables...> tables;
};

template <typename... Tables>
auto make_database(Connection& hdl, Tables&&... tables)
{
  return Database<Tables...>{hdl, std::forward<Tables>(tables)...};
}
}

#endif /* !MYSQL_ORM_DATABASE_HPP_ */
