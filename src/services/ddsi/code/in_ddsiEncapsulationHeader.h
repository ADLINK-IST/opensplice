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
#ifndef IN_DDSIENCAPSULATIONHEADER_H_
#define IN_DDSIENCAPSULATIONHEADER_H_

#include "in_commonTypes.h"
#include "in_ddsiDefinitions.h"

#if defined (__cplusplus)
extern "C" {
#endif

/** */
OS_CLASS(in_ddsiEncapsulationHeader);

/** */
OS_STRUCT(in_ddsiEncapsulationHeader)
{
    in_octet header[4];
};

/** return scans the header and returns number of parsed octets, otherwise -1*/
in_long
in_ddsiEncapsulationHeaderInit(
        in_ddsiEncapsulationHeader _this,
        in_octet* buffer,
        os_size_t bufferLength);


/** */
#define in_ddsiEncapsulationHeaderInitFromBuffer(_header,_des) \
    in_ddsiDeserializerParseOctets(_des, ((_header)->header), 4)

/** */
#define in_ddsiEncapsulationHeaderSerializeInstantly(_kind,_flags,_ser) \
    in_ddsiSerializerAppendOctets_4(_ser, (in_octet) ((_kind >> 8) & 0xff), \
                                          (in_octet) (_kind & 0xff), \
                                          (in_octet) ((_flags >> 8) & 0xff), \
                                          (in_octet) (_flags & 0xff))

/** */
#define in_ddsiEncapsulationHeaderGetCodecId(_h) (((_h)->header[0]<< 8) | ((_h)->header[1]))
/** */
#define in_ddsiEncapsulationHeaderGetFlags(_h) (((_h)->header[2]<< 8) | ((_h)->header[3]))

/** */
#define in_ddsiEncapsulationHeaderIsValid(_h) \
    ( \
            (in_ddsiEncapsulationHeaderGetCodecId(_h)==IN_ENCAPSULATION_CDR_BE) || \
            (in_ddsiEncapsulationHeaderGetCodecId(_h)==IN_ENCAPSULATION_CDR_LE) || \
            (in_ddsiEncapsulationHeaderGetCodecId(_h)==IN_ENCAPSULATION_PL_CDR_BE) || \
            (in_ddsiEncapsulationHeaderGetCodecId(_h)==IN_ENCAPSULATION_PL_CDR_LE)  )
/** */
#define in_ddsiEncapsulationHeaderIsCdr(_h) \
    ( \
            (in_ddsiEncapsulationHeaderGetCodecId(_h)==IN_ENCAPSULATION_CDR_BE) || \
            (in_ddsiEncapsulationHeaderGetCodecId(_h)==IN_ENCAPSULATION_CDR_LE) )
/** */
#define in_ddsiEncapsulationHeaderIsBigEndian(_h) \
    (in_ddsiEncapsulationHeaderGetCodecId(_h)==IN_ENCAPSULATION_CDR_BE || \
        in_ddsiEncapsulationHeaderGetCodecId(_h)==IN_ENCAPSULATION_PL_CDR_BE)

#if defined (__cplusplus)
}
#endif


#endif /* IN_DDSIENCAPSULATIONHEADER_H_ */
