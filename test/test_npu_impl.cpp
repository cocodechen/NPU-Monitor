#include "npu_impl.h"
#include <iostream>
#include <iomanip>

int main()
{
    std::cout << "=== NPU Implementation Test ===" << std::endl;
    try
    {
        // 1. 创建NPUImpl 对象
        NPUImpl npu;
        std::cout << "Collector name: " << npu.name() << std::endl;
        
        // 2. 获取设备标签
        std::cout << "\nGetting device labels..." << std::endl;
        auto labels = npu.labels();
        if (labels.empty())
        {
            std::cout << "No NPU devices found!" << std::endl;
            return 0;
        }
        std::cout << "Found " << labels.size() << " NPU device(s):" << std::endl;
        for (size_t i = 0; i < labels.size(); i++) {
            std::cout << "  Device " << i << ": Card=" << labels[i].card_id 
                      << ", Device=" << labels[i].device_id << std::endl;
        }
        
        // 3. 采集指标数据
        std::cout << "\nSampling metrics..." << std::endl;
        auto metrics = npu.sample();
        std::cout << "\n=== Metrics for " << metrics.size() << " device(s) ===" << std::endl;
        
        for (size_t i = 0; i < metrics.size(); i++)
        {
            std::cout << "\n--- Device " << i << " (Card=" << labels[i].card_id 
                      << ", Device=" << labels[i].device_id << ") ---" << std::endl;
            const auto& m = metrics[i];
        
            // 利用率
            std::cout << "Utilization:" << std::endl;
            std::cout << "  AICore: " << m.util_aicore << "%" << std::endl;
            std::cout << "  AICPU:  " << m.util_aicpu << "%" << std::endl;
            std::cout << "  Memory: " << m.util_mem << "%" << std::endl;
            // 频率
            std::cout << "Frequency:" << std::endl;
            std::cout << "  AICore: " << m.aicore_freq << " MHz" << std::endl;
            std::cout << "  AICPU:  " << m.aicpu_freq << " MHz" << std::endl;
            std::cout << "  Memory: " << m.mem_freq << " MHz" << std::endl;
            // 功耗
            std::cout << std::fixed << std::setprecision(1);
            std::cout << "Power: " << m.power << " W" << std::endl;
            // 其他指标
            std::cout << "Other:" << std::endl;
            std::cout << "  Health:      " << std::hex << "0x" << m.health << std::dec << std::endl;
            // 显示健康状态含义
            std::cout << "    (0: OK, 1: WARN, 2: ERROR, 3: CRITICAL, 0xFFFFFFFF: NOT_EXIST)" << std::endl;
            std::cout << "  Temperature: " << m.temperature << " °C" << std::endl;
            std::cout << "  Voltage:     " << m.voltage << " V" << std::endl;
        }
        std::cout << "\n=== Test completed successfully ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred!" << std::endl;
        return 1;
    }
    
    return 0;
}