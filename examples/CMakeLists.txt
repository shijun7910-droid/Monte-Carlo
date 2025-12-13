# 示例程序
add_executable(basic_simulation basic_simulation.cpp)
add_executable(multi_currency multi_currency.cpp)
add_executable(risk_analysis risk_analysis.cpp)

# 包含目录
target_include_directories(basic_simulation PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(multi_currency PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(risk_analysis PRIVATE ${CMAKE_SOURCE_DIR}/include)

# 链接库（需要先编译主项目）
target_link_libraries(basic_simulation)
target_link_libraries(multi_currency)
target_link_libraries(risk_analysis)
