#ifndef NPU_COLLECTOR_H
#define NPU_COLLECTOR_H

#include <prometheus/gauge.h>
#include <prometheus/registry.h>
#include <prometheus/exposer.h>
#include <prometheus/text_serializer.h>

#include "npu_impl.h"

//创建一个全局的registry
namespace {
    std::shared_ptr<prometheus::Registry> global_registry = std::make_shared<prometheus::Registry>();
}

/*prometheus模板化的NPU收集器--调用底层采集接口impl采到的数据，上传给prometheus*/
/*模板类全部在头文件中实现*/
template<typename T>
class NPUCollector 
{
public:
    /*构造函数：注册Prometheus指标*/
    NPUCollector(T& impl) : impl_(impl)
    {
        //创建指标族（Families）
        auto& registry = *global_registry;
        //AICore利用率
        aicore_util_gauge_ = &prometheus::BuildGauge()
            .Name("npu_aicore_utilization_percent")
            .Help("NPU AI Core utilization percentage")
            .Register(registry);
            
        //AICPU利用率
        aicpu_util_gauge_ = &prometheus::BuildGauge()
            .Name("npu_aicpu_utilization_percent")
            .Help("NPU AI CPU utilization percentage")
            .Register(registry);
            
        //Mem利用率
        mem_util_gauge_ = &prometheus::BuildGauge()
            .Name("npu_memory_utilization_percent")
            .Help("NPU memory utilization percentage")
            .Register(registry);

        //AICore频率
        aicore_freq_gauge_ = &prometheus::BuildGauge()
            .Name("npu_aicore_frequency_mhz")
            .Help("NPU AI Core frequency in MHz")
            .Register(registry);
            
        //AICPU频率
        aicpu_freq_gauge_ = &prometheus::BuildGauge()
            .Name("npu_aicpu_frequency_mhz")
            .Help("NPU AI CPU frequency in MHz")
            .Register(registry);
            
        //Mem频率
        mem_freq_gauge_ = &prometheus::BuildGauge()
            .Name("npu_mem_frequency_mhz")
            .Help("NPU mem frequency in MHz")
            .Register(registry);

        //功耗
        power_gauge_ = &prometheus::BuildGauge()
            .Name("npu_power_watts")
            .Help("NPU power consumption in watts")
            .Register(registry);

        //健康状态
        health_gauge_ = &prometheus::BuildGauge()
            .Name("npu_health") 
            .Help("NPU device health status (0:OK,1:WARN,2:ERROR,3:CRITICAL,0xFFFFFFFF:NOT_EXIST)")
            .Register(registry);
            
        //温度
        temperature_gauge_ = &prometheus::BuildGauge()
            .Name("npu_temperature_celsius")
            .Help("NPU temperature in Celsius")
            .Register(registry);
            
        //电压
        voltage_gauge_ = &prometheus::BuildGauge()
            .Name("npu_voltage_volts")
            .Help("NPU voltage in Volts")
            .Register(registry);
    }
    
    /*收集数据并更新Prometheus指标*/
    void collect()
    {
        // 获取标签（设备列表）和指标数据
        auto label_list = impl_.labels();
        auto metric_list = impl_.sample();
        
        // 更新每个设备的指标
        for (size_t i = 0; i < label_list.size(); i++)
        {
            const auto& label = label_list[i];
            const auto& metric = metric_list[i];
            
            // 构建标签
            std::map<std::string, std::string> labels = {
                {"card_id", std::to_string(label.card_id)},
                {"device_id", std::to_string(label.device_id)}
            };
            
            //更新各个指标 - 使用Add()获取或创建带有标签的指标
            aicore_util_gauge_->Add(labels).Set(metric.util_aicore);
            aicpu_util_gauge_->Add(labels).Set(metric.util_aicpu);
            mem_util_gauge_->Add(labels).Set(metric.util_mem);

            aicore_freq_gauge_->Add(labels).Set(metric.aicore_freq);
            aicpu_freq_gauge_->Add(labels).Set(metric.aicpu_freq);
            mem_freq_gauge_->Add(labels).Set(metric.mem_freq);

            power_gauge_->Add(labels).Set(metric.power);

            health_gauge_->Add(labels).Set(metric.health);
            temperature_gauge_->Add(labels).Set(metric.temperature);
            voltage_gauge_->Add(labels).Set(metric.voltage);
        }
    }
    
    // 获取registry，用于exposer
    static std::shared_ptr<prometheus::Registry> GetRegistry() {
        return global_registry;
    }
    
private:
    T& impl_;  //硬件实现引用
    
    //Prometheus指标 - 使用Family<prometheus::Gauge>类型
    prometheus::Family<prometheus::Gauge>* aicore_util_gauge_;
    prometheus::Family<prometheus::Gauge>* aicpu_util_gauge_;
    prometheus::Family<prometheus::Gauge>* mem_util_gauge_;

    prometheus::Family<prometheus::Gauge>* aicore_freq_gauge_;
    prometheus::Family<prometheus::Gauge>* aicpu_freq_gauge_;
    prometheus::Family<prometheus::Gauge>* mem_freq_gauge_;

    prometheus::Family<prometheus::Gauge>* power_gauge_;

    prometheus::Family<prometheus::Gauge>* health_gauge_;
    prometheus::Family<prometheus::Gauge>* temperature_gauge_;
    prometheus::Family<prometheus::Gauge>* voltage_gauge_;
};

#endif // NPU_COLLECTOR_H