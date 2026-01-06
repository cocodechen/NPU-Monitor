// npu_test.cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>
#include <atomic>

#include "npu_impl.h"
#include "npu_collector.h"
#include <prometheus/exposer.h>


// 控制程序运行
std::atomic<bool> running{true};

void signalHandler(int signal) {
    running = false;
}

int main() {
    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    try {
        // 1. 初始化NPU
        NPUImpl npu_impl;
        
        // 2. 创建收集器（自动注册指标）
        NPUCollector<NPUImpl> collector(npu_impl);
        
        // 3. 启动HTTP服务器
        // 创建exposer
        prometheus::Exposer exposer{"0.0.0.0:8080"};

        // 获取registry并设置给exposer
        exposer.RegisterCollectable(NPUCollector<NPUImpl>::GetRegistry());
        
        std::cout << "NPU监控已启动，访问 http://localhost:8080/metrics 查看数据" << std::endl;
        
        // 4. 定期采集
        while (running) {
            collector.collect();
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}