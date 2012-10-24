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
#ifndef IN_ENDIANNESS__H
#define IN_ENDIANNESS__H

#if defined (__cplusplus)
extern "C" {
#endif


/*  --------- inline macros --------- */

#define IN_UINT16_SWAP_LE_BE(val)		((os_ushort) ( \
    (((os_ushort) (val) & (os_ushort) 0x00ffU) << 8) | \
    (((os_ushort) (val) & (os_ushort) 0xff00U) >> 8)))

#define IN_UINT32_SWAP_LE_BE(val)		((os_uint32) ( \
    (((os_uint32) (val) & (os_uint32) 0x000000ffU) << 24) | \
    (((os_uint32) (val) & (os_uint32) 0x0000ff00U) <<  8) | \
    (((os_uint32) (val) & (os_uint32) 0x00ff0000U) >>  8) | \
    (((os_uint32) (val) & (os_uint32) 0xff000000U) >> 24)))

#ifdef HAVE_IN_INT64
#define IN_UINT64_SWAP_LE_BE(val)         ((os_uint64) ( \
    (((os_uint64) (val) & (os_uint64) 0x00000000000000ffU) << 56) | \
    (((os_uint64) (val) & (os_uint64) 0x000000000000ff00U) << 40) | \
    (((os_uint64) (val) & (os_uint64) 0x0000000000ff0000U) << 24) | \
    (((os_uint64) (val) & (os_uint64) 0x00000000ff000000U) <<  8) | \
    (((os_uint64) (val) & (os_uint64) 0x000000ff00000000U) >>  8) | \
    (((os_uint64) (val) & (os_uint64) 0x0000ff0000000000U) >> 24) | \
    (((os_uint64) (val) & (os_uint64) 0x00ff000000000000U) >> 40) | \
    (((os_uint64) (val) & (os_uint64) 0xff00000000000000U) >> 56)))
#endif


#if defined (__cplusplus)
}
#endif

#endif /* IN_ENDIANNESS__H */
