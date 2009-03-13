#ifndef D_DELETEDATA_H
#define D_DELETEDATA_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_deleteData(s) ((d_deleteData)(s))

d_deleteData    d_deleteDataNew     (d_admin admin,
                                     d_timestamp actionTime,
                                     const c_char* partitionExpr,
                                     const c_char* topicExpr);

void            d_deleteDataFree    (d_deleteData deleteData);

#if defined (__cplusplus)
}
#endif

#endif /*D_DELETEDATA_H*/
