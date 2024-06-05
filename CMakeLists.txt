cmake_minimum_required(VERSION 3.15)
project(mymuduo)

# 项目最终编译成动态库的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
#设置调试信息
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# 定义参与编译的源文件
aux_source_directory(./code SRC_LIST)
# 编译生成动态库
add_library(mymuduo SHARED ${SRC_LIST})

# 此作测试用
add_executable(server main.cc ${SRC_LIST})
target_link_libraries(server pthread)