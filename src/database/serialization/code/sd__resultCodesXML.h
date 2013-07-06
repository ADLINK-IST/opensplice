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
/** \file services/serialization/code/sd__resultCodesXML.h
 *  \brief Macro definitions for XML deserialization validation error codes
 */

#ifndef SD__RESULTCODESXML_H
#define SD__RESULTCODESXML_H

#include "sd__resultCodes.h"

/* Internal error numbers which might occur for the XML serializer */

#define SD_ERRNO_UNEXPECTED_OPENING_TAG      100U
#define SD_MESSAGE_UNEXPECTED_OPENING_TAG    "Unexpected opening tag"

#define SD_ERRNO_UNEXPECTED_CLOSING_TAG      101U
#define SD_MESSAGE_UNEXPECTED_CLOSING_TAG    "Unexpected closing tag"

#define SD_ERRNO_INVALID_REFERENCE_FORMAT    102U
#define SD_MESSAGE_INVALID_REFERENCE_FORMAT  "Invalid reference format"

#define SD_ERRNO_INVALID_CHAR_FORMAT         103U
#define SD_MESSAGE_INVALID_CHAR_FORMAT       "Invalid character format"

#define SD_ERRNO_INVALID_WCHAR_FORMAT        104U
#define SD_MESSAGE_INVALID_WCHAR_FORMAT      "Invalid wide character format"

#define SD_ERRNO_INVALID_SHORT_FORMAT        105U
#define SD_MESSAGE_INVALID_SHORT_FORMAT      "Invalid short format"

#define SD_ERRNO_INVALID_USHORT_FORMAT       106U
#define SD_MESSAGE_INVALID_USHORT_FORMAT     "Invalid unsigned short format"

#define SD_ERRNO_INVALID_LONG_FORMAT         107U
#define SD_MESSAGE_INVALID_LONG_FORMAT       "Invalid long format"

#define SD_ERRNO_INVALID_ULONG_FORMAT        108U
#define SD_MESSAGE_INVALID_ULONG_FORMAT      "Invalid unsigned long format"

#define SD_ERRNO_INVALID_LONGLONG_FORMAT     109U
#define SD_MESSAGE_INVALID_LONGLONG_FORMAT   "Invalid long long format"

#define SD_ERRNO_INVALID_ULONGLONG_FORMAT    110U
#define SD_MESSAGE_INVALID_ULONGLONG_FORMAT  "Invalid unsigned long long format"

#define SD_ERRNO_INVALID_FLOAT_FORMAT        111U
#define SD_MESSAGE_INVALID_FLOAT_FORMAT      "Invalid float format"

#define SD_ERRNO_INVALID_DOUBLE_FORMAT       112U
#define SD_MESSAGE_INVALID_DOUBLE_FORMAT     "Invalid double format"

#define SD_ERRNO_INVALID_BOOLEAN_FORMAT      113U
#define SD_MESSAGE_INVALID_BOOLEAN_FORMAT    "Invalid boolean format"

#define SD_ERRNO_INVALID_OCTET_FORMAT        114U
#define SD_MESSAGE_INVALID_OCTET_FORMAT      "Invalid octet format"

#define SD_ERRNO_INVALID_STRING_FORMAT       115U
#define SD_MESSAGE_INVALID_STRING_FORMAT     "Invalid string format"

#define SD_ERRNO_INVALID_ENUMERATION         116U
#define SD_MESSAGE_INVALID_ENUMERATION       "Invalid enumeration value"
 
#define SD_ERRNO_INVALID_ADDRESS_FORMAT      117U
#define SD_MESSAGE_INVALID_ADDRESS_FORMAT    "Invalid xml format"

#define SD_ERRNO_INVALID_TAG_FORMAT          118U
#define SD_MESSAGE_INVALID_TAG_FORMAT        "Invalid tag format"

#define SD_ERRNO_INVALID_DATA_FORMAT         119U
#define SD_MESSAGE_INVALID_DATA_FORMAT       "Invalid data format"

#define SD_ERRNO_UNEXPECTED_DATA             120U
#define SD_MESSAGE_UNEXPECTED_DATA           "Unexpected data"

#define SD_ERRNO_INVALID_XML_FORMAT          121U
#define SD_MESSAGE_INVALID_XML_FORMAT        "Invalid xml format"


#endif /* SD__RESULTCODESXML_H */
