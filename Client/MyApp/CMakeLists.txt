cmake_minimum_required(VERSION 3.12)
project(MyApp C)

set(CMAKE_C_STANDARD 99)

add_executable(MyApp main.c tc_error.c tc_error.h tc_math.c tc_math.h tc_state.c tc_state.h tc_config.c tc_config.h tc_httpclient.c tc_httpclient.h tc_json.c tc_json.h)
target_link_libraries(MyApp curl m)