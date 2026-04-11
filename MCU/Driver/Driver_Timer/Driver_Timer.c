#include "Driver_Timer.h"

//TODO: Systick 延时函数实现
void SysTick_Delay_us(uint32_t us)
{
    // 此处实现微秒级延时函数
    // 具体实现取决于系统时钟频率和 SysTick 配置
    // 下面是一个简单的示例，假设系统时钟为 72MHz
    uint32_t startTick = SysTick->VAL;
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    while ((startTick - SysTick->VAL) < ticks);
}

void SysTick_Delay_ms(uint32_t ms)
{
    // 此处实现毫秒级延时函数
    // 可以调用微秒延时函数
    for (uint32_t i = 0; i < ms; i++) {
        SysTick_Delay_us(1000);
    }
}

