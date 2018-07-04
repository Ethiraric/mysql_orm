#ifndef MYSQL_ORM_DATABASE_HPP_
#define MYSQL_ORM_DATABASE_HPP_

#include <memory>
#include <tuple>

#include <mysql/mysql.h>

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

  template <typename Model>
  auto select()
  {
    return this->getTable<Model>().select(*this->handle);
  }

  template <auto Attr, auto... Attrs>
  auto select()
  {
    static_assert(
        meta::AllSame_v<meta::AttributeModelGetter_t<decltype(Attr)>,
                        meta::AttributeModelGetter_t<decltype(Attrs)>...>,
        "Attributes do not refer to the same model");
    using Model_t = meta::AttributeModelGetter_t<decltype(Attr)>;
    return this->getTable<Model_t>.template select<Attr, Attrs...>(
        *this->handle);
  }

  template <typename Model>
  auto insert(Model const& model)
  {
    return this->getTable<Model>().insert(*this->handle, &model);
  }

  template <auto Attr,
            auto... Attrs,
            typename Model = meta::AttributeModelGetter_t<decltype(Attr)>>
  auto insert(Model const& model)
  {
    static_assert(
        meta::AllSame_v<meta::AttributeModelGetter_t<decltype(Attr)>,
                        meta::AttributeModelGetter_t<decltype(Attrs)>...>,
        "Attributes do not refer to the same model");
    using Model_t = meta::AttributeModelGetter_t<decltype(Attr)>;
    return this->getTable<Model_t>().template insert<Attr, Attrs...>(
        *this->handle, &model);
  }

  template <auto Attr,
            auto... Attrs,
            typename Model = meta::AttributeModelGetter_t<decltype(Attr)>>
  auto insertAllBut(Model const& model)
  {
    static_assert(
        meta::AllSame_v<meta::AttributeModelGetter_t<decltype(Attr)>,
                        meta::AttributeModelGetter_t<decltype(Attrs)>...>,
        "Attributes do not refer to the same model");
    using Model_t = meta::AttributeModelGetter_t<decltype(Attr)>;
    return this->getTable<Model_t>()
        .template insertAllBut<Attr, Attrs...>(*this->handle, &model);
  }

  template <typename Model>
  auto update()
  {
    using Table_t = typename meta::
        FindMapped<meta::TableModelGetter, Model, Tables...>::type;
    static_assert(!std::is_same_v<Table_t, void>,
                  "Failed to find table for model");
    return Update<std::remove_reference_t<Table_t>>{
        *this->handle, std::get<Table_t>(this->tables)};
  }

  void recreate()
  {
    for_each_tuple(this->tables, [&](auto const& table) {
      this->execute("DROP TABLE IF EXISTS `" + table.getName() + '`');
      this->execute(table.getSchema());
    });
  }

private:
  template <typename Model>
  auto& getTable()
  {
    using Table_t = typename meta::
        FindMapped<meta::TableModelGetter, Model, Tables...>::type;
    static_assert(!std::is_same_v<Table_t, void>,
                  "Failed to find table for model");
    return std::get<Table_t>(this->tables);
  }

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
