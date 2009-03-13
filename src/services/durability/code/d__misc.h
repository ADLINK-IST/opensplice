
#ifndef D__MISC_H
#define D__MISC_H

#include "d__types.h"
#include "c_time.h"
#include "c_base.h"
#include "os.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct baseFind {
    c_base base;
};

void                d_printState                (d_durability durability, 
                                                 d_configuration config, 
                                                 const char* threadName);

void                d_findBaseAction            (v_entity entity,
                                                 c_voidp args);

#if defined (__cplusplus)
}
#endif

#endif /* D__MISC_H */
