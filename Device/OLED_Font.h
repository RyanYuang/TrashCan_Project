#ifndef __OLED_FONT_H
#define __OLED_FONT_H

#include <stdint.h>

// extern 声明：告诉编译器数组存在，但定义在 .c 中
extern const uint8_t OLED_F8x16[95][16];  // 95 字符 × 16 字节 (8x16 字体)

#endif
