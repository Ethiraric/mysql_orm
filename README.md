# mysql_orm
A C++ ORM for MySQL, inspired by [sqlite_orm](https://github.com/fnc12/sqlite_orm).

`mysql_orm` manages your queries to a MySQL database with an intuitive syntax and as much type-checking as possible at compile-time (hence reducing risks that a query is ill-formed).

Consider the following structure:

```cpp
struct Record
{
  uint32_t id;
  int i;
  std::string s;
};
```

We can create a table from this structure the following way:

```cpp
auto table_records = make_table("records",
                                make_column<&Record::id>("id"),
                                make_column<&Record::i>("i"),
                                make_column<&Record::s>("s"));
```

`mysql_orm` automatically deduces the types of the fields and the one of the structure (which we call the _model_).
If the fields do not refer to the same model, an error is raised at compile-time.

We can now make a database connection from the table:

```cpp
auto database = make_database("localhost", 3306, "username", "password", "database", table_records);
```

Any number of tables may be supplied when constructing a database.
Any model whose table is given to the database may be used when performing a query with the `database` object.
Note that all tables must refer to distinct models.

# Performing queries

A `SELECT` query is written the following way:

```cpp
std::vector<Records> records = database.select<Record>()();
```

The database automatically chooses the table for `Record` on which to perform the query.
By default all fields are selected.
Some fields only may be selected by giving as template arguments the pointer to the attributes to select:

```cpp
std::vector<Records> records = database.select<&Record::id, &Record::s>()();
```

This is the same as:

```sql
SELECT `id`, `s` FROM `records`
```

## Adding clauses
SQL clauses are chained using `operator()`s:

```cpp
database.select<Record>()(Where{c<&Record::i>{} = 3})(Limit<1>{})();
```

is equivalent to:

```sql
SELECT * FROM `records` WHERE `records`.`i`=3 LIMIT 1
```

## `c` and `ref`
In order to build conditions correctly for `WHERE` and assignments for `SET`, you need to user one of the `c` and `ref` classes.

`c` is templated on a pointer to an attribute.
When put in an operation, it will be substituted by the column corresponding to the attribute.
In the above example, `c<&Record::i>{}` was changed to `records.i`.

`ref` takes a reference to the variable it is given.

Let us look at the following:

```cpp
int i = 4;

auto query = database.select<Record>()(Where{c<&Record::i>{}=ref{i});
query();
i = 2;
query();
```

The first call to `query` translates to
```sql
SELECT * FROM `records` WHERE `records`.`i`=4
```

While the second translates to
```sql
SELECT * FROM `records` WHERE `records`.`i`=2
```

Had we written `c<&Record::i>{}=i`, both queries would have evaluated with `i=4`.

# Known bugs
 * Does not work with `clang`.

# Roadmap
 * Constraints on multiple columns (`UNIQUE(a, b)`).
