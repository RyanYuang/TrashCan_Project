#ifndef __AHT30_LOG_H
#define __AHT30_LOG_H

#include <stdio.h>
#include "aht30.h"

#define AHT30_LOG_ERROR(format, ...) printf(format, ##__VA_ARGS__)

static const char* aht30_status_to_string(AHT30_Status_Enum status) {
    switch (status) {
        case AHT30_Status_OK: return "OK";
        case AHT30_Status_Error: return "Error";
        case AHT30_Status_Timeout: return "Timeout";
        case AHT30_Status_Busy: return "Busy";
        case AHT30_Status_Uninitialized: return "Uninitialized";
        default: return "Unknown";
    }
}

#endif // __AHT30_LOG_H
