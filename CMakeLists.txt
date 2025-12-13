cmake_minimum_required(VERSION 3.10)
project(CurrencyMonteCarlo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置包含目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# 源文件列表
set(SOURCES
    main.cpp
    src/MonteCarlo.cpp
    src/RandomGenerator.cpp
    src/CurrencyModel.cpp
    src/Statistics.cpp
)

# 可执行文件
add_executable(currency_mc ${SOURCES})

# 在Windows上可能需要链接数学库
if(WIN32)
    target_link_libraries(currency_mc)
endif()
