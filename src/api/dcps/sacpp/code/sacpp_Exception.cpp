/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "os_abstract.h"
#include "sacpp_Exception.h"

DDS::Exception *DDS::Exception::_downcast (DDS::Exception * e)
{
   return e;
}

const DDS::Exception *DDS::Exception::_downcast (const DDS::Exception * e)
{
   return e;
}

DDS::UserException * DDS::Exception::_as_user ()
{
   return 0;
}

DDS::SystemException * DDS::Exception::_as_system ()
{
   return 0;
}
