cmake_minimum_required(VERSION 2.6)

#configuration
project("mysql_orm")

#sources files
set(SRCS
  main.cpp
  test_Column.cpp
  test_ColumnTags.cpp
  test_Database.cpp
  test_Delete.cpp
  test_Insert.cpp
  test_Limit.cpp
  test_Pack.cpp
  test_RemoveOccurences.cpp
  test_GetAll.cpp
  test_Table.cpp
  test_Update.cpp
  test_Varchar.cpp
  test_Where.cpp
)

#binary
add_executable(mysql_orm_tests ${SRCS})
target_include_directories(mysql_orm_tests PRIVATE .)
target_link_libraries(mysql_orm_tests mysqlclient mysql_orm)
add_test(NAME mysql_orm_tests COMMAND mysql_orm_tests)

function(add_failtest testname src passpattern)
  add_executable(${testname} ${src})
  target_include_directories(${testname} PRIVATE .)
  target_link_libraries(${testname} mysqlclient mysql_orm)
  set_target_properties(${testname} PROPERTIES
    EXCLUDE_FROM_ALL TRUE
    EXCLUDE_FROM_DEFAULT_BUILD TRUE)
  add_test(NAME ${testname}
    COMMAND ${PROJECT_SOURCE_DIR}/failcompile_wrapper.sh
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  set_tests_properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION ${passpattern})
  set_tests_properties(${testname} PROPERTIES
    ENVIRONMENT "CMAKE_COMMAND=${CMAKE_COMMAND};FAILTARGET=${testname};CMAKE_CONFIGURATION=$<CONFIGURATION>")
endfunction(add_failtest)

add_failtest(select_no_model fail/test_GetAllNoModel.cpp "Failed to find table for model")
add_failtest(select_attributes_different_models fail/test_GetAllAttributesDifferentModels.cpp "Attributes do not refer to the same model")
add_failtest(select_attribute_not_found fail/test_GetAllAttributeNotFound.cpp "Failed to find attribute")
