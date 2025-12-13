cmake_minimum_required(VERSION 3.10)
project(CurrencyMonteCarlo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    main.cpp
    src/MonteCarlo.cpp
    src/RandomGenerator.cpp
    src/CurrencyModel.cpp
    src/Statistics.cpp
)

# Executable
add_executable(currency_mc ${SOURCES})

# Link math library
if(UNIX)
    target_link_libraries(currency_mc m)
endif()
