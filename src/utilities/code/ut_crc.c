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
#include "ut_crc.h"
#include "os.h"

#define UT_CRC_TABLE_SIZE 256 /* for every possible byte value */

OS_STRUCT(ut_crc) {
    os_uint32 key;
    os_uint32 table[UT_CRC_TABLE_SIZE];
};



/**************************************************************
 * Private functions
 **************************************************************/
static void
ut_crcInit(
    ut_crc crc,
    os_uint32 key)
{
    os_uint32 i;
    os_uint32 j;
    os_uint32 reg;
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
ut_crc
ut_crcNew(
    os_uint32 key)
{
    ut_crc crc;



    crc = os_malloc(sizeof(OS_STRUCT(ut_crc)));
    if (crc) {
        ut_crcInit(crc,key);
    }

    return crc;
}

void
ut_crcFree(
    ut_crc _this)
{
    os_free (_this);
}

os_uint32
ut_crcCalculate(
    ut_crc crc,
    void *buf,
    os_uint32 length)
{
    os_uint32 i;
    os_uchar top;
    os_uint32 reg;
    os_uchar *vptr = (os_uchar*)buf;

    assert(crc);
    assert(buf);

       reg = 0;
       if (crc && buf) {
           i = 0;
           while (i < length) {

               top = (os_uchar)(reg >> 24);
               top ^= *vptr;
               reg = (reg << 8) ^ crc->table[top];
               vptr++;
               i++;
           }
       }
       return reg;
}

/**************************************************************
 * Public functions
 **************************************************************/
