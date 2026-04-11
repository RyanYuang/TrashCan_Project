#ifndef __GY302_REG_H__
#define __GY302_REG_H__

// ********** GY302 Register Addresses ********** //
#define GY302_ADDR_HIGH         0x5C    // 7-bit address with ADDR pin high
#define GY302_ADDR_LOW          0x23    // 7-bit address with ADDR pin low

// ********** GY302 Command Set ********** //
#define GY302_POWER_DOWN        0x00    // No active state
#define GY302_POWER_ON          0x01    // Waiting for measurement command
#define GY302_RESET             0x07    // Reset data register value

// ********** Measurement Modes ********** //
#define GY302_CONTINUOUS_H_RES_MODE     0x10    // Start measurement at 1lx resolution
#define GY302_CONTINUOUS_H_RES_MODE2    0x11    // Start measurement at 0.5lx resolution
#define GY302_CONTINUOUS_L_RES_MODE     0x13    // Start measurement at 4lx resolution
#define GY302_ONE_TIME_H_RES_MODE       0x20    // Start measurement at 1lx resolution once
#define GY302_ONE_TIME_H_RES_MODE2      0x21    // Start measurement at 0.5lx resolution once
#define GY302_ONE_TIME_L_RES_MODE       0x23    // Start measurement at 4lx resolution once

// ********** Change Measurement Time ********** //
#define GY302_CHANGE_MEASUREMENT_TIME_H 0x40    // Change measurement time (High byte)
#define GY302_CHANGE_MEASUREMENT_TIME_L 0x60    // Change measurement time (Low byte)

// Device commit
/*
    Measurement mode:
    H-res mode:     120ms   Resolution: 1lx
    H-res mode2:    120ms  Resolution: 0.5lx
    L-res mode:     16ms    Resolution: 4lx

    Reset:
    onlu for reset data register, not for power on/off,It's used for removing previous data.
    This command is not working in power down mode.
*/


#endif // __GY302_REG_H__