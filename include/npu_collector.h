#ifndef NPU_COLLECTOR_H
#define NPU_COLLECTOR_H

#include <prometheus/gauge.h>
#include <prometheus/registry.h>
#include <prometheus/labels.h>

#include "npu_impl.h"

/*prometheus模板化的NPU收集器--调用底层采集接口impl采到的数据，上传给prometheus*/
/*模板类全部在头文件中实现*/
template<typename T>
class NPUCollector : public Collector
{
public:
    /*构造函数：注册Prometheus指标*/
    NPUCollector(T& impl) : impl_(impl)
    {
        //创建指标族（Families）
        auto& registry = prometheus::BuildPrometheusRegistry();
        //注册各种指标
        //AICore利用率
        aicore_util_family_ = &registry.AddGaugeFamily(
            "npu_aicore_utilization_percent",
            "NPU AI Core utilization percentage"
        );
        //AICPU利用率
        aicpu_util_family_ = &registry.AddGaugeFamily(
            "npu_aicpu_utilization_percent",
            "NPU AI CPU utilization percentage"
        );
        //Mem利用率
        mem_util_family_ = &registry.AddGaugeFamily(
            "npu_memory_utilization_percent",
            "NPU memory utilization percentage"
        );

        //AICore频率
        aicore_freq_family_ = &registry.AddGaugeFamily(
            "npu_aicore_frequency_mhz",
            "NPU AI Core frequency in MHz"
        );
        //AICPU频率
        aicpu_freq_family_ = &registry.AddGaugeFamily(
            "npu_aicpu_frequency_mhz",
            "NPU AI CPU frequency in MHz"
        );
        //Mem频率
        mem_freq_family_ = &registry.AddGaugeFamily(
            "npu_mem_frequency_mhz",
            "NPU mem frequency in MHz"
        );

        //功耗
        power_family_ = &registry.AddGaugeFamily(
            "npu_power_watts",
            "NPU power consumption in watts"
        );

        //健康状态
        health_family_ = &registry.AddGaugeFamily(
            "npu_health", 
            "NPU device health status (0:OK,1:WARN,2:ERROR,3:CRITICAL,0xFFFFFFFF:NOT_EXIST)"
        );
        //温度
        temperature_family_ = &registry.AddGaugeFamily(
            "npu_temperature_celsius",
            "NPU temperature in Celsius"
        );
        //电压
        voltage_family_ = &registry.AddGaugeFamily(
            "npu_voltage_volts",
            "NPU voltage in Volts"
        );
    }
    
    /*收集数据并更新Prometheus指标*/
    void collect() override
    {
        //获取标签（设备列表）和指标数据
        auto label_list = impl_.labels();
        auto metric_list = impl_.sample();
        
        //更新每个设备的指标
        for (size_t i = 0; i < label_list.size(); i++)
        {
            const auto& label = label_list[i];
            const auto& metric = metric_list[i];
            //构建标签对
            prometheus::Labels labels = {
                {"card_id", std::to_string(label.card_id)},
                {"device_id", std::to_string(label.device_id)}
            };
            
            //更新各个指标
            aicore_util_family_->Add(labels).Set(metric.util_aicore);
            aicpu_util_family_->Add(labels).Set(metric.util_aicpu);
            mem_util_family_->Add(labels).Set(metric.util_mem);

            aicore_freq_family_->Add(labels).Set(metric.aicore_freq);
            aicpu_freq_family_->Add(labels).Set(metric.aicpu_freq);
            mem_freq_family_->Add(labels).Set(metric.mem_freq);

            power_family_->Add(labels).Set(metric.power);

            health_family_->Add(labels).Set(metric.health);
            temperature_family_->Add(labels).Set(metric.temperature);
            voltage_family_->Add(labels).Set(metric.voltage);
        }
    }
    
private:
    T& impl_;  //硬件实现引用
    
    // Prometheus指标族
    prometheus::GaugeFamily* aicore_util_family_;
    prometheus::GaugeFamily* aicpu_util_family_;
    prometheus::GaugeFamily* mem_util_family_;

    prometheus::GaugeFamily* aicore_freq_family_;
    prometheus::GaugeFamily* aicpu_freq_family_;
    prometheus::GaugeFamily* mem_freq_family_;

    prometheus::GaugeFamily* power_family_;

    prometheus::GaugeFamily* health_family_;
    prometheus::GaugeFamily* temperature_family_;
    prometheus::GaugeFamily* voltage_family_;
};

#endif // NPU_COLLECTOR_H