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
#include "sacpp_UserException.h"

DDS::UserException * DDS::UserException::_downcast
   (DDS::Exception * e)
{
   return e ? e->_as_user() : 0;
}

const DDS::UserException* DDS::UserException::_downcast 
   (const DDS::Exception * e)
{
   return _downcast ((DDS::Exception*) e);
}

DDS::UserException *DDS::UserException::_as_user()
{
   return this;
}
