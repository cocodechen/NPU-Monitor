#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "dcmi_interface_api.h"    // 昇腾 DCMI 接口头文件


#define MAX_AICPU_CORES 16        // AICPU 最大核数（用于结构体定义）
#define NPU_OK 0                  // DCMI 接口返回成功值


void sleep_ms(int ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}


int main(int argc, char **argv)
{
    /*1.获取参数：采样周期，单位为ms*/
    int interval = 1000;
    if (argc > 1)
    {
        interval = atoi(argv[1]);
        if (interval <= 0) interval = 1000;
    }

    /*2.初始化DCMI库并获取一些信息*/
    int ret = dcmi_init();
    if (ret != NPU_OK)
    {
        printf("dcmi_init failed (ret=%d)\n", ret);
        return ret;
    }
    /*获取版本信息*/
    char dcmi_ver[64] = {0};
    ret = dcmi_get_dcmi_version(dcmi_ver, sizeof(dcmi_ver));
    if(ret !=NPU_OK)
    {
        printf("dcmi_get_dcmi_version failed (ret=%d)\n",ret);
        return ret;
    }
    printf("[DCMI version]：%s\n",dcmi_ver);
    /*获取驱动版本*/
    char driver_ver[64] = {0};
    ret = dcmi_get_driver_version(driver_ver, sizeof(driver_ver));
    if (ret!=NPU_OK)
    {
        printf("dcmi_get_driver_version failed (ret=%d)\n", ret);
        return ret;
    }
    printf("[DCMI Driver Version]: %s\n",driver_ver);
    

    /*3.获取卡列表*/
    int card_count = 0;
    int card_list[MAX_CARD_NUM] = {0};
    ret = dcmi_get_card_list(&card_count, card_list, MAX_CARD_NUM);
    if (ret != NPU_OK)
    {
        printf("dcmi_get_card_list failed (ret=%d)\n", ret);
        return ret;
    }
    if(card_count==0)
    {
        printf("[WARNING] No Ascend devices found\n");
        return 0;
    }
    printf("[Card Num]：%d\n", card_count);

    /*4.主循环：周期性采集数据*/
    while (1)
    {
        /*遍历每个卡*/
        for (int i = 0; i < card_count; i++)
        {
            int card = card_list[i];
            /*获取该卡上的设备数量*/
            int device_count = 0, mcu_id = 0, cpu_id = 0;
            ret = dcmi_get_device_id_in_card(card, &device_count, &mcu_id, &cpu_id);
            if (ret != NPU_OK)
            {
                printf("Card %d: get_device_id_in_card failed (ret=%d)\n", card, ret);
                continue;
            }
            printf("[CardID] %d, [Device Num]: %d\n",card,device_count);
            /*遍历每个卡上的设备，即卡上的多个NPU芯片*/
            for (int dev = 0; dev < device_count; dev++)
            {
                printf("[Device ID] %d\n",dev);
                /*获取设备健康信息*/
                unsigned int health = 0;
                ret = dcmi_get_device_health(card, dev, &health);
                if(ret!=NPU_OK)
                {
                    printf("dcmi_get_device_health failed (ret=%d)\n",ret);
                    continue;
                }
                const char *health_str = "UNKNOWN";
                switch (health)
                {
                    case 0: health_str = "OK"; break;
                    case 1: health_str = "WARN"; break;
                    case 2: health_str = "ERROR"; break;
                    case 3: health_str = "CRITICAL"; break;
                    case 0xFFFFFFFF: health_str = "NOT_EXIST"; break;
                }
                printf("[Health]：%s(0x%x)\n",health_str, health);

                /*获取功耗信息（单位：0.1W）*/
                int power = 0;
                ret = dcmi_get_device_power_info(card, dev, &power);
                if (ret != NPU_OK)
                {
                    printf("get_device_power_info failed (ret=%d)\n",ret);
                }
                else printf("[Power]：%.1fW\n",power/10.0);

                /*获取AI Core频率信息*/
                struct dcmi_aicore_info aicore = {0};
                ret = dcmi_get_device_aicore_info(card, dev, &aicore);
                if (ret != NPU_OK)
                {
                    printf("get_device_aicore_info failed (ret=%d)\n",ret);
                }
                else printf("[AICore Freqency]：%uMHz\n",aicore.cur_freq);

                /*获取AI CPU频率*/
                struct dcmi_aicpu_info aicpu = {0};
                ret = dcmi_get_device_aicpu_info(card, dev, &aicpu);
                if (ret != NPU_OK)
                {
                    printf("get_device_aicpu_info failed (ret=%d)\n",ret);
                }
                else printf("[AICPU Frequency]：%uMHz\n",aicpu.cur_freq);

                /*获取 AI Core、AICPU、内存等利用率*/
                unsigned int util_aicore = 0, util_aicpu = 0, util_mem = 0;
                /*input_type=2: AI Core*/
                ret = dcmi_get_device_utilization_rate(card, dev, 2, &util_aicore);
                if (ret != NPU_OK)
                {
                    printf("get_device_utilization_rate(AI Core) failed (ret=%d)\n",ret);
                }
                else printf("[AICore Utilization Rate]：%u%%\n",util_aicore);
                /*input_type=3: AICPU*/
                ret = dcmi_get_device_utilization_rate(card, dev, 3, &util_aicpu);
                if (ret != NPU_OK)
                {
                    printf("get_device_utilization_rate(AICPU) failed (ret=%d)\n",ret);
                }
                else printf("[AICPU Utilization Rate]：%u%%\n",util_aicpu);
                /*input_type=1: 内存（显存）*/
                ret = dcmi_get_device_utilization_rate(card, dev, 1, &util_mem);
                if (ret != NPU_OK)
                {
                    printf("get_device_utilization_rate(Mem) failed (ret=%d)\n",ret);
                }
                else printf("[Mem Utilization Rate]：%u%%\n",util_mem);

                /*获取温度（摄氏度）*/
                int temperature = 0;
                ret = dcmi_get_device_temperature(card, dev, &temperature);
                if (ret != NPU_OK)
                {
                    printf("get_device_temperature failed (ret=%d)\n",ret);
                }
                else printf("[Temperature]：%dC\n",temperature);

                /*获取电压（单位：0.01V）*/
                unsigned int voltage = 0;
                ret = dcmi_get_device_voltage(card, dev, &voltage);
                if (ret != NPU_OK)
                {
                    printf("get_device_voltage failed (ret=%d)\n",ret);
                }
                else printf("[Voltage]：%.2fV\n",voltage/100.0);
            }
        }
        /*等待下一个采样周期*/
        sleep_ms(interval);
    }

    return 0;
}
