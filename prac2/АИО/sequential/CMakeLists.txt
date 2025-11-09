cmake_minimum_required(VERSION 3.16)

project(sequential_simulated_annealing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SA_SOURCES
    src/main.cpp
    src/annealing/SimulatedAnnealing.cpp
    src/cooling/BoltzmannCooling.cpp
    src/cooling/CauchyCooling.cpp
    src/cooling/CoolingFactory.cpp
    src/cooling/LogarithmicCooling.cpp
    src/mutation/SchedulingMutation.cpp
    src/solution/SchedulingSolution.cpp
    src/util/JobDataLoader.cpp
)

add_executable(sequential_app ${SA_SOURCES})

target_include_directories(sequential_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_options(sequential_app PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
)

