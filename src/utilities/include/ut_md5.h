/* Minimal changes introduced, for which:
 *                         
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
/*
  Copyright (C) 1999, 2002 Aladdin Enterprises.  All rights reserved.

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  L. Peter Deutsch
  ghost@aladdin.com

 */
/* $Id: md5.h,v 1.4 2002/04/13 19:20:28 lpd Exp $ */
/*
  Independent implementation of MD5 (RFC 1321).

  This code implements the MD5 Algorithm defined in RFC 1321, whose
  text is available at
        http://www.ietf.org/rfc/rfc1321.txt
  The code is derived from the text of the RFC, including the test suite
  (section A.5) but excluding the rest of Appendix A.  It does not include
  any code or documentation that is identified in the RFC as being
  copyrighted.

  The original and principal author of md5.h is L. Peter Deutsch
  <ghost@aladdin.com>.  Other authors are noted in the change history
  that follows (in reverse chronological order):

  2002-04-13 lpd Removed support for non-ANSI compilers; removed
        references to Ghostscript; clarified derivation from RFC 1321;
        now handles byte order either statically or dynamically.
  1999-11-04 lpd Edited comments slightly for automatic TOC extraction.
  1999-10-18 lpd Fixed typo in header comment (ansi2knr rather than md5);
        added conditionalization for C++ compilation from Martin
        Purschke <purschke@bnl.gov>.
  1999-05-03 lpd Original version.
 */

#ifndef UT_MD5_H
#define UT_MD5_H

#include "os_defs.h"

/*
 * This package supports both compile-time and run-time determination of CPU
 * byte order.  If ARCH_IS_BIG_ENDIAN is defined as 0, the code will be
 * compiled to run only on little-endian CPUs; if ARCH_IS_BIG_ENDIAN is
 * defined as non-zero, the code will be compiled to run only on big-endian
 * CPUs; if ARCH_IS_BIG_ENDIAN is not defined, the code will be compiled to
 * run on either big- or little-endian CPUs, but will run slightly less
 * efficiently on either one than if ARCH_IS_BIG_ENDIAN is defined.
 */

typedef unsigned char ut_md5_byte_t; /* 8-bit byte */
typedef unsigned int ut_md5_word_t; /* 32-bit word */

/* Define the state of the MD5 Algorithm. */
typedef struct ut_md5_state_s {
    ut_md5_word_t count[2];        /* message length in bits, lsw first */
    ut_md5_word_t abcd[4];         /* digest buffer */
    ut_md5_byte_t buf[64];         /* accumulate block */
} ut_md5_state_t;

#ifdef __cplusplus
extern "C" 
{
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/* Initialize the algorithm. */
OS_API void ut_md5_init(ut_md5_state_t *pms);

/* Append a string to the message. */
OS_API void ut_md5_append(ut_md5_state_t *pms, const ut_md5_byte_t *data, unsigned nbytes);

/* Finish the message and return the digest. */
OS_API void ut_md5_finish(ut_md5_state_t *pms, ut_md5_byte_t digest[16]);

#undef OS_API

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif /* UT_MD5_H */
