#ifndef SACPP_LOCALOBJECT_H
#define SACPP_LOCALOBJECT_H

#include "sacpp_Object.h"
#include "sacpp_if.h"

class SACPP_API DDS_DCPS::LocalObject : virtual public DDS_DCPS::Object
{
   friend void DDS_DCPS::release(DDS_DCPS::LocalObject * p);

public:

   LocalObject();

   virtual void _add_ref();
   virtual void _remove_ref();
   virtual DDS_DCPS::Boolean _is_a(const char * id);
   virtual DDS_DCPS::ULong _hash(DDS_DCPS::ULong maximum);

   virtual DDS_DCPS::Boolean _is_equivalent(const DDS_DCPS::Object * obj);

   virtual DDS_DCPS::Boolean is_local_object();

protected:

   virtual ~LocalObject ();
};

#undef SACPP_API
#endif /* SACPP_LOCALOBJECT_H */
