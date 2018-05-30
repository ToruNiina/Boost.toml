cmake_minimum_required(VERSION 3.0)
enable_testing()
project(boost_toml)

include_directories(${PROJECT_SOURCE_DIR})

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/test/toml")
    execute_process(COMMAND git submodule update --init --recursive
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()

find_package(Boost)
if(${Boost_FOUND})
    include_directories(${Boost_INCLUDE_DIRS})
else()
    message(FATAL "If Boost exists somewhere CMake does not look by default, try")
    message(FATAL "$ cmake path/to/CMakeLists.txt -DBOOST_ROOT=/path/to/boost")
    message(FATAL "currently, BOOST_ROOT = ${BOOST_ROOT}")
endif()

add_subdirectory(test)
add_subdirectory(sample)
