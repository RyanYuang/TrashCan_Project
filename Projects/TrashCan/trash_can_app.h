/**
 ******************************************************************************
 * @file    trash_can_app.h
 * @brief   垃圾桶项目应用头文件
 ******************************************************************************
 */

#ifndef __TRASH_CAN_APP_H__
#define __TRASH_CAN_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ========================= 应用接口 ========================= */

/**
 * @brief 垃圾桶应用主函数
 * @note 在main()中调用此函数启动应用
 */
void TrashCan_App_Main(void);

/**
 * @brief 应用生命周期回调函数
 */
bool TrashCan_OnInit(void);
void TrashCan_OnStart(void);
void TrashCan_OnLoop(void);
void TrashCan_OnStop(void);
void TrashCan_OnError(uint32_t error_code);

#ifdef __cplusplus
}
#endif

#endif /* __TRASH_CAN_APP_H__ */
