# 设置包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

# 添加所有源文件
file(GLOB_RECURSE SOURCES "*.cpp")

# 创建可执行文件
add_executable(currency_simulator main.cpp ${SOURCES})

# 设置属性
set_target_properties(currency_simulator PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# 链接库
target_link_libraries(currency_simulator Threads::Threads)

if(OpenMP_CXX_FOUND)
    target_link_libraries(currency_simulator OpenMP::OpenMP_CXX)
endif()
