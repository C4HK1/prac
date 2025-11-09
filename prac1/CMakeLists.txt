cmake_minimum_required(VERSION 3.20)
project(mafia_sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
  ${SRC_DIR}/*.cpp
)

add_executable(mafia ${SOURCES})

target_include_directories(mafia PRIVATE ${INCLUDE_DIR})

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(mafia stdc++fs)
endif()
