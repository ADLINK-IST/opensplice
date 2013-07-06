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

#include <sys/stat.h>

int remove (const char *filename)
{
    struct stat sbuff;
    if (stat(filename, &sbuff) < 0)
    {
        return (-1);
    }
    if (S_ISDIR (sbuff.st_mode))
    {
        return (rmdir (filename));
    }
    else
    {
        return (unlink (filename));
    }
}
