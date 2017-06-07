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
#ifndef PA_INLINE_H
#define PA_INLINE_H

/* We want to inline these, but we don't want to emit an exernally
   visible symbol for them and we don't want warnings if we don't use
   them.

   It appears as if a plain "inline" will do just that in C99.

   In traditional GCC one had to use "extern inline" to achieve that
   effect, but that will cause an externally visible symbol to be
   emitted by a C99 compiler.

   Starting with GCC 4.3, GCC conforms to the C99 standard if
   compiling in C99 mode, unless -fgnu89-inline is specified. It
   defines __GNUC_STDC_INLINE__ if "inline"/"extern inline" behaviour
   is conforming the C99 standard.

   So: GCC >= 4.3: choose between "inline" & "extern inline" based
   upon __GNUC_STDC_INLINE__; for GCCs < 4.2, rely on the traditional
   GCC behaiour; and for other compilers assume they behave conforming
   the standard if they advertise themselves as C99 compliant (use
   "inline"), and assume they do not support the inline keywords
   otherwise.

   GCC when not optimizing ignores "extern inline" functions. So we
   need to distinguish between optimizing & non-optimizing ... */

/* Defining PA_HAVE_INLINE is a supported way of overruling this file */
#ifndef PA_HAVE_INLINE

#if __STDC_VERSION__ >= 199901L
#  /* C99, but old GCC nonetheless doesn't implement C99 semantics ... */
#  if __GNUC__ && ! defined __GNUC_STDC_INLINE__
#    define PA_HAVE_INLINE 1
#    define PA_INLINE extern __inline__
#  else
#    define PA_HAVE_INLINE 1
#    define PA_INLINE inline
#  endif
#elif defined __STDC__ && defined __GNUC__ && ! defined __cplusplus
#  if __OPTIMIZE__
#    if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)
#      ifdef __GNUC_STDC_INLINE__
#        define PA_HAVE_INLINE 1
#        define PA_INLINE __inline__
#      else
#        define PA_HAVE_INLINE 1
#        define PA_INLINE extern __inline__
#      endif
#    else
#      define PA_HAVE_INLINE 1
#      define PA_INLINE extern __inline__
#    endif
#  endif
#endif

#if ! PA_HAVE_INLINE
#define PA_INLINE
#endif

#endif /* not defined PA_HAVE_INLINE */

#endif /* PA_INLINE_H */
