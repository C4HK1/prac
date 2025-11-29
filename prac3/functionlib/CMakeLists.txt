cmake_minimum_required(VERSION 3.14)

project(FunctionLibrary LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(FUNCTIONLIB_PUBLIC_HEADERS
    include/functionlib/functionlib_export.h
    include/functionlib/TFunction.h
    include/functionlib/FunctionFactory.h
    include/functionlib/FunctionOperators.h
    include/functionlib/GradientDescent.h
)

add_subdirectory(src/functions)

add_library(FunctionLibrary SHARED
    src/TFunction.cpp
    src/FunctionFactory.cpp
    src/Operators.cpp
    src/GradientDescent.cpp
    ${FUNCTIONLIB_PUBLIC_HEADERS}
    ${FUNCTIONLIB_FUNCTIONS_SOURCES}
)

target_include_directories(FunctionLibrary
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(FunctionLibrary PRIVATE FUNCTIONLIB_LIBRARY)

enable_testing()

add_executable(FunctionLibraryTests tests/FunctionLibraryTests.cpp)
target_link_libraries(FunctionLibraryTests PRIVATE FunctionLibrary GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(FunctionLibraryTests)

