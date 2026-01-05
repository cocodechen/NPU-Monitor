###############################################################################
# CMake基本配置
###############################################################################

# CMake最低版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(npu_monitor)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

###############################################################################
# 头文件包含设置 - 告诉编译器在哪里找头文件
###############################################################################

# 设置头文件搜索路径
# 相当于 g++ 的 -I 参数
include_directories(
    ${CMAKE_SOURCE_DIR}/include                     # -I. (当前目录，实际是 -Iinclude)
    /usr/local/Ascend/driver/include               # -I/usr/local/Ascend/driver/include
)
#只要头文件在 include_directories()指定的目录中，就可以在cpp文件中直接用 #include "文件名.h" 包含

# 设置库文件搜索路径
# 相当于 g++ 的 -L 参数
link_directories(
    /usr/local/Ascend/driver/lib64/driver          # -L/usr/local/Ascend/driver/lib64/driver
)

###############################################################################
# 创建核心库（将所有的依赖编译为一个库，包括要链接的库、自己写的.h和.cpp）
###############################################################################

# 收集所有源文件创建库
add_library(npu_core STATIC
    src/npu_impl.cpp
)

# 链接华为库到核心库
target_link_libraries(npu_core
    dcmi
    ascend_hal
)

# 设置库输出目录
set_target_properties(npu_core PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

###############################################################################
# 生成可执行文件
###############################################################################
#test_npu_impl
add_executable(test_npu_impl
    test/test_npu_impl.cpp
)
# 链接核心库
target_link_libraries(test_npu_impl npu_core)
# 设置可执行文件输出目录
set_target_properties(test_npu_impl PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

#test_npu_collecter
add_executable(test_npu_collecter
    test/test_npu_collecter.cpp
)
# 链接核心库
target_link_libraries(test_npu_collecter npu_core)
# 设置可执行文件输出目录
set_target_properties(test_npu_collecter PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

###############################################################################
# 输出信息
###############################################################################
message(STATUS "项目: ${PROJECT_NAME}")
message(STATUS "输出目录: ${CMAKE_BINARY_DIR}/lib")