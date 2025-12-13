# src/CMakeLists.txt

# 源文件列表
set(SOURCE_FILES
    main.cpp
    models/GBM.cpp
    models/Vasicek.cpp
    models/HullWhite.cpp
    random/NormalGenerator.cpp
    random/SobolGenerator.cpp
    statistics/Statistics.cpp
    statistics/RiskMetrics.cpp
    statistics/Convergence.cpp
    simulation/MonteCarloSimulator.cpp
    simulation/PathGenerator.cpp
    simulation/ResultAnalyzer.cpp
    utils/CSVWriter.cpp
    utils/JSONConfig.cpp
    utils/Timer.cpp
    utils/Logger.cpp
)

# 创建可执行文件
add_executable(currency_simulator ${SOURCE_FILES})

# 链接库
target_link_libraries(currency_simulator PRIVATE Threads::Threads)

# 添加包含目录
target_include_directories(currency_simulator PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)

# 如果使用OpenMP
if(WITH_OPENMP AND OpenMP_CXX_FOUND)
    target_link_libraries(currency_simulator PRIVATE OpenMP::OpenMP_CXX)
endif()
