
#ifndef D__GROUP_H
#define D__GROUP_H

#include "d__types.h"
#include "v_group.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_group){
    C_EXTENDS(d_object);
    d_topic topic;
    d_partition partition;
    d_durabilityKind kind;
    d_completeness completeness;
    d_quality quality;
    v_group vgroup;
    c_ulong storeCount;
    c_bool private;
};

void    d_groupDeinit   (d_object object);

#define d_group(g) ((d_group)(g))

#if defined (__cplusplus)
}
#endif

#endif /* D__GROUP_H */
