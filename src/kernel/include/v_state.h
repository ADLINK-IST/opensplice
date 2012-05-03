/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

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
#define L_SYNCHRONOUS      (0x0001U << 15) /* 32768 */
#define L_TRANSACTION      (0x0001U << 16) /* 65536 */

/*
 * Sets all bits in state that are set in mask.
 *
 * Example:
 *      state = 101101;
 *      v_stateSet(state, 000011);
 *      state == 101111;
 *
 */
#define v_stateSet(state,mask)    ((state)|=(mask))

/*
 * Clears all bits in state that are set in mask.
 *
 * Example:
 *      state = 101111;
 *      v_stateClear(state, 000011);
 *      state == 101100;
 *
 */
#define v_stateClear(state,mask)  ((state)&=(~mask))

/*
 * Tests whether the supplied mask is set. If all bits in the mask are set in
 * the state, then the result is TRUE.
 *
 * Example:
 *      v_stateTest(101001, 000101) == FALSE
 *      v_stateTest(101001, 001001) == TRUE
 *      v_stateTest(101001, 101001) == TRUE
 */
#define v_stateTest(state,mask)   (((state)&(mask))==(mask))

/*
 * Tests whether the supplied mask is NOT set. If all bits in the mask are not
 * set in the state, then the result is TRUE.
 *
 * Example:
 *      v_stateTestNot(101001, 000110) == TRUE
 *      v_stateTestNot(101001, 000010) == TRUE
 *      v_stateTestNot(101001, 001110) == FALSE
 *
 * Note that this is different than calling v_stateTest with a negated mask:
 *      v_stateTest(101001, ~000110) == FALSE
 *      v_stateTestNot(101001, 000110) == TRUE
 * Or negating the output of v_stateTest:
 *      !v_stateTest(101001, 001110) == TRUE
 *      v_stateTestNot(101001, 001110) == FALSE
 * Or negating the output of v_stateTest with a negated mask:
 *      !v_stateTest(101001, ~001110) == TRUE
 *      v_stateTestNot(101001, 001110) == FALSE
 */
#define v_stateTestNot(state,mask)(((state)&(~mask))==(state))

/*
 * Returns a value != 0 when (at least) one of the bits set in mask is also set
 * in state. The result is a bitmask of the bits that where set both in state
 * and mask.
 *
 * Example:
 *      v_stateTestOr(101001, 100001) != 0
 *      v_stateTestOr(101001, 010010) == 0
 */
#define v_stateTestOr(state,mask) ((state)&(mask))

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
