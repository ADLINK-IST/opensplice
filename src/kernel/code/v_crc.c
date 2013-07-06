/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "v__crc.h"

/**************************************************************
 * Private functions
 **************************************************************/
static void
v_crcInit(
    v_crc crc,
    c_ulong key)
{
    c_ulong i;
    c_ulong j;
    c_ulong reg;
    int topBit;

    assert(crc);

    crc->key = key;

    /* calculate table based on key */
    for (i = 0; i < 256; ++i) {
        /* put this byte at the top of the register*/
        reg = i << 24;
        /* for all bits in a byte*/
        for (j = 0; j < 8; ++j) {
            topBit = (reg & 0x80000000) != 0;
            reg <<= 1;
            if (topBit) {
                reg ^= crc->key;
            }
        }
        crc->table[i] = reg;
    }
}

/**************************************************************
 * Protected functions
 **************************************************************/
v_crc
v_crcNew(
    v_kernel k,
    c_ulong key)
{
    v_crc crc;
    c_type type;

    assert(k);

    if (k) {

        type = c_resolve(c_getBase(k), "kernelModule::v_crc");
        assert(type);
        if (type) {
            crc = c_new(type);
            c_free(type);
            if (crc) {
                v_crcInit(crc,key);
            }
        } else {
            crc = NULL;
        }
    } else {
        crc = NULL;
    }

    return crc;
}

c_ulong
v_crcCalculate(
    v_crc crc,
    const c_char *buf,
    c_ulong length)
{
    c_ulong i;
    c_ulong top;
    c_ulong reg;

    assert(crc);
    assert(buf);

    reg = 0;
    if (crc && buf) {
        i = 0;
        while (i < length) {
            top = reg >> 24;
            top ^= (c_ulong)*buf;
            reg = (reg << 8) ^ crc->table[top];
            buf++;
            i++;
        }
    }
    return reg;
}

/**************************************************************
 * Public functions
 **************************************************************/
