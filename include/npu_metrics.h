#ifndef NPU_METRICS_H
#define NPU_METRICS_H

/*标签结构体（用于标识设备）*/
struct NPULabel
{
    int card_id;
    int device_id;
};

/*指标数据结构体*/
struct NPUMetric
{
    //利用率（%）
    uint32_t util_aicore; //AICore
    uint32_t util_aicpu; //AICPU
    uint32_t util_mem; //Mem
    //频率（MHz）
    uint32_t aicore_freq; //AICore
    uint32_t aicpu_freq; //AICPU
    uint32_t mem_freq; //Mem
    //功耗（W）
    double power;

    //其他
    //健康状态 (0: OK, 1: WARN, 2: ERROR, 3: CRITICAL, 0xFFFFFFFF: NOT_EXIST)
    uint32_t health;
    //温度（C）
    int32_t temperature;
    //电压（V）
    double voltage;
};

#endif