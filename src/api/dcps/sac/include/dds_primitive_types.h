/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef DDS_PRIMITIVE_TYPES_H
#define DDS_PRIMITIVE_TYPES_H

#if defined (__cplusplus)
extern "C" {
#endif

typedef short int                           DDS_short;
typedef int                                 DDS_long;
typedef long long int                       DDS_long_long;
typedef unsigned short int                  DDS_unsigned_short;
typedef unsigned int                        DDS_unsigned_long;
typedef unsigned long long int              DDS_unsigned_long_long;
typedef float                               DDS_float;
typedef double                              DDS_double;
typedef long double                         DDS_long_double;
typedef char                                DDS_char;
typedef unsigned char                       DDS_octet;
typedef unsigned char                       DDS_boolean;
typedef DDS_char *                          DDS_string;
typedef void *                              DDS_Object;

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef NULL
#define NULL 0
#endif


#if defined (__cplusplus)
}
#endif

#endif /* DDS_PRIMITIVE_TYPES_H */
