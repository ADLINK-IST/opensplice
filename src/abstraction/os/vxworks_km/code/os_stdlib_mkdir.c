/*
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
/** \file os/vxworks5.5/code/os_stdlib_mkdir.c
 *  \brief wrapper to solve interface differents
 *
 */

os_int32
os_mkdir(
    const char *path,
    mode_t mode)
{
#if defined (OSPL_VXWORKS653)
   return -1;
#else
   return (mkdir(path));
#endif
}
