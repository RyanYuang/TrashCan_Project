#include <stddef.h>

#include "OELD_New.h"
#include "OLED_Font.h"

#define OLED_CONTROL_CMD   0x00
#define OLED_CONTROL_DATA  0x40
#define OLED_FONT_WIDTH    8u
#define OLED_FONT_HEIGHT   16u

struct OLED_Device_Driver {
    I2C_Platform_Structure i2c; /* Bit-bang I2C driver */
    uint8_t address;            /* 7-bit device address */
};

static OLED_Device_Driver oled_device_drivers[OLED_MAX_NUM];

static OLED_Device_Driver* get_driver(const OLED_Handle* handler)
{
    if (handler == NULL || handler->ID >= OLED_MAX_NUM) {
        return NULL;
    }
    if (oled_device_drivers[handler->ID].address == 0) {
        return NULL;
    }
    return &oled_device_drivers[handler->ID];
}

static OLED_Status_Enum oled_write_bytes(OLED_Device_Driver* dev, uint8_t control, const uint8_t* data, uint8_t len)
{
    if (dev == NULL || data == NULL || len == 0) {
        return OLED_Status_Error;
    }

    return (IIC_Write_Reg(&dev->i2c, dev->address, control, len, data) == IIC_OK)
        ? OLED_Status_OK
        : OLED_Status_Error;
}

static OLED_Status_Enum oled_write_command(OLED_Device_Driver* dev, uint8_t cmd)
{
    return oled_write_bytes(dev, OLED_CONTROL_CMD, &cmd, 1);
}

static OLED_Status_Enum oled_write_data(OLED_Device_Driver* dev, const uint8_t* data, uint8_t len)
{
    return oled_write_bytes(dev, OLED_CONTROL_DATA, data, len);
}

static OLED_Status_Enum oled_set_cursor(OLED_Device_Driver* dev, uint8_t page, uint8_t column)
{
    if (page >= 8 || column >= 128) {
        return OLED_Status_Error;
    }

    if (oled_write_command(dev, 0xB0 | page) != OLED_Status_OK) {
        return OLED_Status_Error;
    }
    if (oled_write_command(dev, 0x00 | (column & 0x0F)) != OLED_Status_OK) {
        return OLED_Status_Error;
    }
    if (oled_write_command(dev, 0x10 | (column >> 4)) != OLED_Status_OK) {
        return OLED_Status_Error;
    }

    return OLED_Status_OK;
}

static uint32_t oled_pow10(uint8_t exponent)
{
    uint32_t result = 1;
    while (exponent--) {
        result *= 10;
    }
    return result;
}

OLED_Status_Enum OLED_Device_Detection(OLED_Handle* handler)
{
	return (IIC_Scan(&oled_device_drivers[handler->ID].i2c) > 0)? OLED_Status_OK : OLED_Status_Error;
}

OLED_Status_Enum OLED_Device_Create(OLED_Handle* handler, Pin_Struct* SDA, Pin_Struct* SCL, uint8_t address)
{
    if (handler == NULL || SDA == NULL || SCL == NULL || address == 0) {
        return OLED_Status_Error;
    }

    for (int i = 0; i < OLED_MAX_NUM; i++) {
        if (oled_device_drivers[i].address == 0) {
            IIC_Register(&oled_device_drivers[i].i2c, SDA, SCL);	//	IIC 设备注册
            oled_device_drivers[i].address = address;				// 绑定设备地址
            handler->ID = (uint16_t)i;								// 绑定设备ID
            return OLED_Status_OK;
        }
    }
    return OLED_Status_Error;
}

OLED_Status_Enum OLED_IIC_Write_Reg(OLED_Device_Driver* dev, uint8_t reg, uint8_t data)
{
    return oled_write_bytes(dev, reg, &data, 1);
}

OLED_Status_Enum OLED_IIC_Read_Reg(OLED_Device_Driver* dev, uint8_t reg, uint8_t* data)
{
    if (dev == NULL || data == NULL) {
        return OLED_Status_Error;
    }
    return (IIC_Read_Reg(&dev->i2c, dev->address, reg, 1, data) == IIC_OK)
        ? OLED_Status_OK
        : OLED_Status_Error;
}

OLED_Status_Enum OLED_Init(OLED_Handle* handler)
{
    OLED_Device_Driver* dev = get_driver(handler);
    if (dev == NULL) {
        return OLED_Status_Error;
    }

    /* Basic SSD1306 init sequence for 128x64 */
    uint8_t init_cmds[] = {
        0xAE,       /* Display off */
        0xD5, 0x80, /* Set display clock divide ratio/oscillator frequency */
        0xA8, 0x3F, /* Multiplex ratio */
        0xD3, 0x00, /* Display offset */
        0x40,       /* Display start line */
        0x8D, 0x14, /* Charge pump */
        0x20, 0x00, /* Memory addressing mode: horizontal */
        0xA1,       /* Segment remap */
        0xC8,       /* COM scan direction */
        0xDA, 0x12, /* COM pins hardware configuration */
        0x81, 0xCF, /* Contrast control */
        0xD9, 0xF1, /* Pre-charge period */
        0xDB, 0x40, /* VCOMH deselect level */
        0xA4,       /* Entire display on resume */
        0xA6,       /* Normal display */
        0x2E,       /* Deactivate scroll */
        0xAF        /* Display on */
    };

    for (size_t i = 0; i < sizeof(init_cmds); i++) {
        if (oled_write_command(dev, init_cmds[i]) != OLED_Status_OK) {
            return OLED_Status_Error;
        }
    }

    return OLED_Status_OK;
}

OLED_Status_Enum OLED_Clear(OLED_Handle* handler)
{
    OLED_Device_Driver* dev = get_driver(handler);
    if (dev == NULL) {
        return OLED_Status_Error;
    }

    const uint8_t blank[16] = {0};

    for (uint8_t page = 0; page < 8; page++) {
        if (oled_set_cursor(dev, page, 0) != OLED_Status_OK) {
            return OLED_Status_Error;
        }

        for (uint8_t col = 0; col < 128; col += sizeof(blank)) {
            if (oled_write_data(dev, blank, (uint8_t)sizeof(blank)) != OLED_Status_OK) {
                return OLED_Status_Error;
            }
        }
    }
    return OLED_Status_OK;
}

OLED_Status_Enum OLED_ShowChar(OLED_Handle* handler, uint8_t x, uint8_t y, char chr)
{
    OLED_Device_Driver* dev = get_driver(handler);
    if (dev == NULL) {
        return OLED_Status_Error;
    }
    if (y > 6 || x > 120) { /* Need two pages for 16-pixel height */
        return OLED_Status_Error;
    }
    if (chr < ' ' || chr > '~') {
        return OLED_Status_Error;
    }

    uint8_t index = (uint8_t)(chr - ' ');
    if (oled_set_cursor(dev, y, x) != OLED_Status_OK) {
        return OLED_Status_Error;
    }
    if (oled_write_data(dev, &OLED_F8x16[index][0], OLED_FONT_WIDTH) != OLED_Status_OK) {
        return OLED_Status_Error;
    }
    if (oled_set_cursor(dev, (uint8_t)(y + 1), x) != OLED_Status_OK) {
        return OLED_Status_Error;
    }
    if (oled_write_data(dev, &OLED_F8x16[index][OLED_FONT_WIDTH], OLED_FONT_WIDTH) != OLED_Status_OK) {
        return OLED_Status_Error;
    }

    return OLED_Status_OK;
}

OLED_Status_Enum OLED_ShowString(OLED_Handle* handler, uint8_t x, uint8_t y, const char* str, uint8_t font_size)
{
    if (font_size != OLED_FONT_HEIGHT) { /* Only 8x16 font is available */
        return OLED_Status_Error;
    }
    OLED_Device_Driver* dev = get_driver(handler);
    if (dev == NULL || str == NULL) {
        return OLED_Status_Error;
    }

    uint8_t cursor_x = x;
    uint8_t cursor_y = y;

    for (const char* p = str; *p != '\0'; p++) {
        if (cursor_x > 120) {
            cursor_x = 0;
            cursor_y = (uint8_t)(cursor_y + 2);
        }
        if (cursor_y > 6) {
            return OLED_Status_Error;
        }
        if (OLED_ShowChar(handler, cursor_x, cursor_y, *p) != OLED_Status_OK) {
            return OLED_Status_Error;
        }
        cursor_x = (uint8_t)(cursor_x + OLED_FONT_WIDTH);
    }

    return OLED_Status_OK;
}

OLED_Status_Enum OLED_ShowNum(OLED_Handle* handler, uint8_t x, uint8_t y, uint32_t num, uint8_t len)
{
    if (len == 0 || len > 10) {
        return OLED_Status_Error;
    }

    uint8_t cursor_x = x;
    for (uint8_t i = 0; i < len; i++) {
        uint32_t divisor = oled_pow10((uint8_t)(len - i - 1));
        char digit = (char)((num / divisor) % 10U + '0');
        if (OLED_ShowChar(handler, cursor_x, y, digit) != OLED_Status_OK) {
            return OLED_Status_Error;
        }
        cursor_x = (uint8_t)(cursor_x + OLED_FONT_WIDTH);
    }

    return OLED_Status_OK;
}

