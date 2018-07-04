cmake_minimum_required(VERSION 2.6)

#configuration
project("mysql_orm")

#sources files
set(SRCS
  main.cpp
  test_Column.cpp
  test_ColumnTags.cpp
  test_Database.cpp
  test_Insert.cpp
  test_Limit.cpp
  test_Pack.cpp
  test_RemoveOccurences.cpp
  test_Select.cpp
  test_Table.cpp
  test_Update.cpp
  test_Varchar.cpp
  test_Where.cpp
)

#binary
add_executable(mysql_orm_tests ${SRCS})
target_include_directories(mysql_orm_tests PRIVATE .)
target_link_libraries(mysql_orm_tests mysqlclient mysql_orm)
