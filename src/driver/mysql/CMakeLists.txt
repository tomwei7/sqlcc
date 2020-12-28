add_library(mysqldriver
    STATIC
    dsn.cc
    driver.cc
    conn.cc
    stmt.cc
)

target_include_directories(mysqldriver
    PUBLIC
    ${MARIADB_CONNECTOR_C_DIR}/include
    ${MARIADB_CONNECTOR_C_BUILD_DIR}/include
)

target_link_libraries(mysqldriver mariadbclient)

add_library(sqlcc::mysqldriver ALIAS mysqldriver)
