get_target_property(MYSQL_DRIVER_INCLUDE_DIRECTORIES sqlcc::mysqldriver INCLUDE_DIRECTORIES)

add_executable(mysqldriver_test mysqldriver_test.cc)
target_link_libraries(mysqldriver_test 
    PRIVATE sqlcc::mysqldriver gtest_main
)

target_include_directories(mysqldriver_test PRIVATE ${MYSQL_DRIVER_INCLUDE_DIRECTORIES})

add_test(NAME mysqldriver_test COMMAND mysql_driver_test)
