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
#ifndef SD_STRING_H
#define SD_STRING_H

#include "c_base.h"


C_CLASS(sd_string);

sd_string
sd_stringNew (
    c_ulong size);

void
sd_stringFree (
    sd_string str);

void
sd_stringAdd (
    sd_string     str,
    const c_char *format,
    ...);

const c_char *
sd_stringContents (
    sd_string str);


#endif
