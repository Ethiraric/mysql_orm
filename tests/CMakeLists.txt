cmake_minimum_required(VERSION 2.6)

#configuration
project("demangler")

#includes directories
include_directories(
  ../include
  ./
)

#sources files
set(SRCS
  main.cpp
  test_Column.cpp
  test_ColumnTags.cpp
  test_Table.cpp
)

#binary
add_executable(mysql_orm_tests ${SRCS})
