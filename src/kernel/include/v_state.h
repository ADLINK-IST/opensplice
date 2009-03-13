
#ifndef V_STATE_H
#define V_STATE_H

#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_node</code> cast method.
 *
 * This method casts an object to a <code>v_node</code> object.
 * Before the cast is performed, if the NDEBUG flag is not set,
 * the type of the object is checked to be <code>v_node</code> or
 * one of its subclasses.
 */
#define v_node(n)      (C_CAST(n,v_node))

#define v_nodeState(n) (v_node(n)->nodeState)

#define L_WRITE            (0x0001U << 0) /* 1 */
#define L_NEW              (0x0001U << 1) /* 2 */
#define L_DISPOSED         (0x0001U << 2) /* 4 */
#define L_NOWRITERS        (0x0001U << 3) /* 8 */
#define L_INCOMPLETE       (0x0001U << 4) /* 16 */
#define L_READ             (0x0001U << 5) /* 32 */
#define L_EMPTY            (0x0001U << 6) /* 64 */
#define L_LAZYREAD         (0x0001U << 7)  /* 128 this sample state value is used for lazy read state */
#define L_REGISTER         (0x0001U << 8) /* 256 */
#define L_UNREGISTER       (0x0001U << 9) /* 512 */
#define L_RESEND           (0x0001U << 10) /* 1024 */
#define L_IMPLICIT         (0x0001U << 11) /* 2048 */
#define L_SUSPENDED        (0x0001U << 12) /* 4096 */
#define L_STATECHANGED     (0x0001U << 13) /* 8192 */
#define L_VALIDDATA        (0x0001U << 14) /* 16384 */

#define v_stateSet(state,mask)    ((state)|=(mask))
#define v_stateClear(state,mask)  ((state)&=(~mask))
#define v_stateTest(state,mask)   (((state)&(mask))==(mask))
#define v_stateTestOr(state,mask) ((state)&(mask))

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
