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

#include <string.h>

#include "os_uniqueNodeId.h"
#include "os_sha2.h"
#include "os_time.h"
#include "os_stdlib.h"


os_uint32 os_uniqueNodeIdGet()
{
   /* DNS name is max 253 Chars */
   char hostnamebuf[256];
   os_time currenttime;
   char digest[SHA256_DIGEST_STRING_LENGTH];
   SHA256_CTX context;
   unsigned int i1, i2, i3, i4, i5, i6, i7, i8;

   os_gethostname(hostnamebuf, sizeof(hostnamebuf));
   currenttime=os_timeGet();

   os_SHA256Init(&context);
   os_SHA256Update(&context, (unsigned char *)&currenttime, sizeof(currenttime));
   os_SHA256Update(&context, (unsigned char *)hostnamebuf, strlen(hostnamebuf));
   os_SHA256End(&context, digest);
   sscanf(digest, "%8x%8x%8x%8x%8x%8x%8x%8x",
          &i1, &i2, &i3, &i4, &i5, &i6, &i7, &i8 );
   /* Mask top bit from unique id, until other fixes are available */
   return( (i1^i2^i3^i4^i5^i6^i7^i8) & 0x7fffffffU );
}
