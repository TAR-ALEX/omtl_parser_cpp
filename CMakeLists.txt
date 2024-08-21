cmake_minimum_required(VERSION 3.16)
project(omtl_parser) # this has to be first, otherwise bad things happen

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

include_directories(
    "./vendor/include/"
    "./include/"
    "./src"
    "./vendor/src"
)

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "./src/main.cpp" "./include/*" "./vendor/src/*" "./vendor/include/*")
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "./src/test.cpp" "./include/*" "./vendor/src/*" "./vendor/include/*")

add_executable("${PROJECT_NAME}_main" ${APP_SOURCES})
add_executable("${PROJECT_NAME}_test" ${TEST_SOURCES})

target_link_libraries("${PROJECT_NAME}_main" PRIVATE)   
target_link_libraries("${PROJECT_NAME}_test" PRIVATE)   



