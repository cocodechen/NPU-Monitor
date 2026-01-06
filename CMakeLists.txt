###############################################################################
# CMake 基本配置
###############################################################################
# CMake 最低版本
cmake_minimum_required(VERSION 3.10)
# 项目名
project(npu_monitor)
# C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


###############################################################################
# 创建核心库
###############################################################################
### npu监测
add_library(npu_core STATIC
    src/npu_impl.cpp
)
target_include_directories(npu_core
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        /usr/local/Ascend/driver/include
)
target_link_directories(npu_core
    PUBLIC
        /usr/local/Ascend/driver/lib64/driver   
)
target_link_libraries(npu_core
    PRIVATE
        dcmi
        ascend_hal
)

### prometheus
find_package(prometheus-cpp CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
add_library(prometheus_deps INTERFACE)
target_link_libraries(prometheus_deps
    INTERFACE
        prometheus-cpp::core
        prometheus-cpp::pull
        ZLIB::ZLIB
)

###############################################################################
# 可执行文件
###############################################################################
### test_npu_impl
add_executable(test_npu_impl
    test/test_npu_impl.cpp
)
target_link_libraries(test_npu_impl
    PRIVATE
        npu_core
)
set_target_properties(test_npu_impl PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

### test_npu_collector
add_executable(test_npu_collector
    test/test_npu_collector.cpp
)
target_link_libraries(test_npu_collector
    PRIVATE
        npu_core
        prometheus_deps
)
set_target_properties(test_npu_collector PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

###############################################################################
# 构建信息输出
###############################################################################
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Build dir: ${CMAKE_BINARY_DIR}")
