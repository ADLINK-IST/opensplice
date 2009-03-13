
#ifndef U__CFDATA_H
#define U__CFDATA_H

#include "u__cfNode.h"
#include "u_cfData.h"

#include "v_cfData.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(u_cfData) {
    C_EXTENDS(u_cfNode);
};

#define u_cfData(o) ((u_cfData)(o))

u_cfData
u_cfDataNew(
    u_participant participant,
    v_cfData kData);

#if defined (__cplusplus)
}
#endif

#endif /* U__CFDATA_H */
