cmake_minimum_required(VERSION 2.6)

#configuration
project("mysql_orm")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib/)

option(MYSQL_ORM_BUILD_TESTS "Build the demangler tests" ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wshadow")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#includes directories
include_directories(
  include/
)

if (MYSQL_ORM_BUILD_TESTS)
  add_subdirectory(tests)
endif()
