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
#include "is.h"
#include <ctype.h>

extern int ishspace (char c)
{
   return((c == ' ') || (c == '\t'));
}

extern int isbsymchar (char c)
{
   return(isalpha((int) c) || (c == '_'));
}

extern int issymchar (char c)
{
   return(isalnum((int) c) || (c == '_') || (c == '$'));
}
