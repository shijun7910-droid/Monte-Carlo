cmake_minimum_required(VERSION 3.10)
project(CurrencyMonteCarlo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 项目选项
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(WITH_OPENMP "Use OpenMP for parallel computing" ON)
option(WITH_JSON "Use JSON for configuration" OFF)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 查找依赖
find_package(Threads REQUIRED)

# OpenMP支持
if(WITH_OPENMP)
    find_package(OpenMP REQUIRED)
    if(OpenMP_CXX_FOUND)
        message(STATUS "OpenMP found: ${OpenMP_CXX_VERSION}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    endif()
endif()

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)

# 主项目
add_subdirectory(src)

# 测试
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# 例子
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# 安装目标
install(TARGETS currency_simulator DESTINATION bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/configs DESTINATION share/currency-monte-carlo)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/scripts DESTINATION share/currency-monte-carlo)
