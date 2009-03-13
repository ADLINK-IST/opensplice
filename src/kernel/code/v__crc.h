#ifndef V__CRC_H
#define V__CRC_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "kernelModule.h"

v_crc
v_crcNew(
    v_kernel k,
    c_ulong key);

c_ulong
v_crcCalculate(
    v_crc _this,
    const c_char *buf,
    c_ulong length);

#if defined (__cplusplus)
}
#endif

#endif /* V__CRC_H */
