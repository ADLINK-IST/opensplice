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
#ifndef CCPP_ORB_ABSTRACTION_H
#define CCPP_ORB_ABSTRACTION_H
  
#include <tao/LocalObject.h>
#include <tao/Valuetype/ValueBase.h>

namespace DDS
{
  #define LOCAL_REFCOUNTED_OBJECT ::TAO_Local_RefCounted_Object
  #define LOCAL_REFCOUNTED_VALUEBASE ::CORBA::DefaultValueRefCountBase

  #define THROW_ORB_EXCEPTIONS ACE_THROW_SPEC ((CORBA::SystemException))
  #define THROW_ORB_AND_USER_EXCEPTIONS(...) ACE_THROW_SPEC ((CORBA::SystemException, __VA_ARGS__))

  #define THROW_VALUETYPE_ORB_EXCEPTIONS
  #define THROW_VALUETYPE_ORB_AND_USER_EXCEPTIONS(...)
}; 

#endif /* ORB_ABSTRACTION */
