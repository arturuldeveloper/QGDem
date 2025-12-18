cmake_minimum_required(VERSION 3.13)
project(Infrared C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(infrared STATIC
    src/infrared.c
    src/ppm_io.c
    src/bmp_io.c
)

target_include_directories(infrared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(infrared_demo src/main.c)
target_link_libraries(infrared_demo PRIVATE infrared)

add_executable(test_infrared tests/test_infrared.c)
target_link_libraries(test_infrared PRIVATE infrared)

enable_testing()
add_test(NAME infrared_basic COMMAND test_infrared)
