
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// time.h for Simple-XX/SimpleKernel.

#ifndef _TIME_H_
#define _TIME_H_

#include "stdint.h"

class time_t {
public:
    // seconds after the minute [0-60]
    uint32_t tm_sec;
    // minutes after the hour [0-59]
    uint32_t tm_min;
    // hours since midnight [0-23]
    uint32_t tm_hour;
    // day of the month [1-31]
    uint32_t tm_mday;
    // months since January [0-11]
    uint32_t tm_mon;
    // years since 1900
    uint32_t tm_year;
    // days since Sunday [0-6]
    uint32_t tm_wday;
    // days since January 1 [0-365]
    uint32_t tm_yday;
    // Daylight Saving Time flag
    int      tm_isdst;
    // offset from UTC in seconds
    long     tm_gmtoff;
    // timezone abbreviation
    char*    tm_zone;

    time_t(void) {
        return;
    }

    ~time_t(void) {
        return;
    }
};

#endif /* _TIME_H_ */
