#include "User_Main.h"

void Error_Handler(void);
void HAL_Delay(uint32_t Delay);
int EnvCar_App_Init(void);
void EnvCar_App_Task(void);

/* Legacy control variables used by protocol layer. */
uint8_t Dircetion = 0;
uint8_t Speed = 0;

void User_Main(void)
{
    if (EnvCar_App_Init() != 0) {
        Error_Handler();
    }

    while (1) {
        EnvCar_App_Task();
        HAL_Delay(50);
    }
}
