cmake_minimum_required(VERSION 3.12)
project(App C)

set(CMAKE_C_STANDARD 99)

add_executable(App main.c tc_error.h tc_error.c tc_state.h tc_state.c tc_math.c tc_math.h)
target_link_libraries(App curl m)
