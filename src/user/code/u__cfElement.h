
#ifndef U__CFELEMENT_H
#define U__CFELEMENT_H

#include "u__cfNode.h"
#include "u_cfElement.h"

#include "v_cfElement.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(u_cfElement) {
    C_EXTENDS(u_cfNode);
};

#define u_cfElement(o) ((u_cfElement)(o))

u_cfElement
u_cfElementNew(
    u_participant participant,
    v_cfElement kElement);

#if defined (__cplusplus)
}
#endif

#endif /* U__CFELEMENT_H */
