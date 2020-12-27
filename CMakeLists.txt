cmake_minimum_required (VERSION 3.17)

project (sqlcc CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

SET(INSTALL_PLUGINDIR ${CMAKE_BINARY_DIR})
add_subdirectory(third-party/mariadb-connector-c)
set(MARIADB_CONNECTOR_C_DIR ${PROJECT_SOURCE_DIR}/third-party/mariadb-connector-c)
set(MARIADB_CONNECTOR_C_BUILD_DIR ${PROJECT_BINARY_DIR}/third-party/mariadb-connector-c)

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
)
#include_directories(
#    ${PROJECT_SOURCE_DIR}/src
#    ${PROJECT_SOURCE_DIR}/src/include
#)
#
#find_package(Threads REQUIRED)
#find_package(OpenSSL REQUIRED)
#
#set(LINK_LIBRARIES ${LIB_MARIADB} Threads::Threads OpenSSL::Crypto OpenSSL::SSL dl)
#
#add_library(sqlcc
#    src/driver.cc
#)
#
#add_library(sqlcc-plugin-mysql
#    src/plugin/mysql/driver.cc
#    src/plugin/mysql/mysql_conn.cc
#)
#
## Now simply link against gtest or gtest_main as needed. Eg
##add_executable(example example.cpp)
##target_link_libraries(example gtest_main)
##add_test(NAME example_test COMMAND example)
#add_subdirectory(test)
add_subdirectory(src)
add_subdirectory(test)
