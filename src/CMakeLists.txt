add_subdirectory(driver)

add_library(sqlcc
    STATIC
    driver.cc
    sqlcc.cc
)

add_library(sqlcc::sqlcc ALIAS sqlcc)
