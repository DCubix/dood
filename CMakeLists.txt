cmake_minimum_required(VERSION 3.12)
project(dood)

file(GLOB SRC "src/*.h" "src/*.c")
add_executable(${PROJECT_NAME} ${SRC})