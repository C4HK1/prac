cmake_minimum_required(VERSION 3.14)

project(FunctionLibraryApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

get_filename_component(FUNCTIONLIB_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../functionlib"
    ABSOLUTE
)

add_subdirectory("${FUNCTIONLIB_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/functionlib_build")

add_executable(FunctionLibDemo main.cpp)
target_link_libraries(FunctionLibDemo PRIVATE FunctionLibrary)


