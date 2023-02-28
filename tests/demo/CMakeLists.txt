cmake_minimum_required(VERSION 2.6)

project (ice_web_demo)

SET(CXX_FLAGS -g2 -ggdb -Wall -Werror)

add_executable(ice_web_demo main.cpp)
target_link_libraries(ice_web_demo iceweb pthread)

