#ifndef _USER_MAIN_H_
#define _USER_MAIN_H_

typedef enum {
    UV_IDLE = 0,         // 空闲（未启动）
    UV_RUNNING,          // 消毒中
    UV_PAUSED,           // 暂停（桶盖开启，自动续时）
    UV_MANUAL_PAUSED,    // 手动暂停（按键/语音关闭，需手动开启）
} UV_State_t;

extern UV_State_t uv_disinfect_state;

void User_Main(void);
void UV_Disinfect_Start(void);
void UV_Disinfect_Stop(void);

extern int full_flag1;
extern int full_flag2;

#endif
