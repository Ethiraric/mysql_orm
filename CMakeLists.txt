cmake_minimum_required(VERSION 2.6)

#configuration
project("mysql_orm")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib/)

option(MYSQL_ORM_BUILD_TESTS "Build the demangler tests" ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wshadow")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(mysql_orm INTERFACE)
target_include_directories(mysql_orm INTERFACE include/)

if (MYSQL_ORM_BUILD_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()
