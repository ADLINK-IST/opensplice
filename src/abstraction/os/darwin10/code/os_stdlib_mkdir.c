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
/** \file os/darwin/code/os_stdlib_mkdir.c
 *  \brief wrapper to solve interface differents
 *
 */

os_int32
os_mkdir(
    const char *path,
    mode_t mode)
{
    return (mkdir(path, mode));
}

