
#ifndef OS__TIME_H
#define OS__TIME_H

#include "os_time.h"

void
os_timeSetUserClock (
    os_time (*userClock)(void)
    );

#endif /* OS__TIME_H */
