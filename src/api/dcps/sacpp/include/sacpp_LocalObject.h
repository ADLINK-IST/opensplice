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
#ifndef SACPP_LOCALOBJECT_H
#define SACPP_LOCALOBJECT_H

#include "sacpp_Object.h"
#include "sacpp_if.h"

class SACPP_API DDS::LocalObject : virtual public DDS::Object
{
   friend void DDS::release(DDS::LocalObject * p);

public:

   LocalObject();

   virtual void _add_ref();
   virtual void _remove_ref();
   virtual DDS::Boolean _is_a(const char * id);
   virtual DDS::ULong _hash(DDS::ULong maximum);

   virtual DDS::Boolean _is_equivalent(const DDS::Object * obj);

   virtual DDS::Boolean is_local_object();

protected:

   virtual ~LocalObject ();
};

#undef SACPP_API
#endif /* SACPP_LOCALOBJECT_H */
