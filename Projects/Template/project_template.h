/**
 ******************************************************************************
 * @file    project_template.h
 * @brief   项目模板头文件
 ******************************************************************************
 */

#ifndef __PROJECT_TEMPLATE_H__
#define __PROJECT_TEMPLATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ========================= 应用接口 ========================= */

/**
 * @brief 项目应用主函数
 * @note 在main()中调用此函数启动应用
 */
void MyProject_App_Main(void);

/**
 * @brief 应用生命周期回调函数
 */
bool MyProject_OnInit(void);
void MyProject_OnStart(void);
void MyProject_OnLoop(void);
void MyProject_OnPause(void);
void MyProject_OnResume(void);
void MyProject_OnStop(void);
void MyProject_OnError(uint32_t error_code);

#ifdef __cplusplus
}
#endif

#endif /* __PROJECT_TEMPLATE_H__ */
