.. _`Evolving data models`:

####################
Evolving data models
####################

It is likely that over time a data model will change; for example, 
new fields with extra information are often added to an existing
data model.

Normally all applications using that data model need to be 
recompiled against the new (changed) data model in order to be aware of the 
extra fields. However, when using a data model based on the GPB system, 
it is possible to add extra fields to the data model and use 
applications based on the original data model *and* applications based on
the new data model in a mixed environment.
 
|caution|
  It is only possible to combine old and new applications 
  with different data models as long as the required fields are the same. 
  Remember, a key field or a filterable field is always required, 
  so these fields can not be changed or added if it is necessary 
  to combine old and new data models.

In our example we will make a new data model with some extra fields:

   - new phonetype: *SKYPE*
   - new phoneNumber property: *secret*
   - new string for an alias: *facebookname*

Proto file with new options::

   import "omg/dds/descriptor.proto";
   package address;
   message Person {
      option (.omg.dds.type); // default type-name will be 'Person'
      required string name = 1 [(.omg.dds.member).key = true];
      required int32 age = 2   [(.omg.dds.member).filterable = true];
      optional string email = 3;

        enum PhoneType {
          UNDEFINED = 0;
          MOBILE = 1;
          HOME = 2;
          WORK = 3;
          SKYPE = 4; // **** added SKYPE phonetype enum-value ****
        }
        message PhoneNumber {
          required string number = 1;
          optional PhoneType type = 2 [default = UNDEFINED];
          optional bool secret = 3 [default = false];
           // **** added a new phoneNumber property ****
        }
     repeated PhoneNumber phone = 4;
     repeated Person friend = 5;
     optional string facebookname = 6 [default = "NONE"];
      // **** added alias facebook name ****
   }

Old publisher and old subscriber
================================

Data printed by subscriber program::

   Name  = Jane Doe
   Age   = 23
   Email = jane.doe@somedomain.com
   Phone = 0123456789 (HOME)
   Friend:
      Name  = John Doe
      Age   = 35
      Email = john.doe@somedomain.com

New publisher and new subscriber
================================

Data printed by subscriber program::

   Name     = Jane Doe
   Facebook = Jane123
   Age      = 23
   Email    = jane.doe@somedomain.com
   Phone    = 0123456789 (HOME) secret=false
   Phone    = 0612345678 (MOBILE) secret=true
   Phone    = splicer (SKYPE) secret=false
   Friend:
      Name     = John Doe
      Facebook = NONE
      Age      = 35
      Email    = john.doe@somedomain.com

Old publisher and new subscriber
================================

New subscriber gets default values for absent fields::

   Name     = Jane Doe
   Facebook = NONE
   Age      = 23
   Email    = jane.doe@somedomain.com
   Phone    = 0123456789 (HOME) secret=false
   Friend:
      Name     = John Doe
      Facebook = NONE
      Age      = 35
      Email    = john.doe@somedomain.com

New publisher and old subscriber
================================

Old subscriber doesn't understand the ``SKYPE`` phonetype
so reverts to the default ``UNDEFINED`` phonetype.

Read data in old subscriber::

   Name  = Jane Doe
   Age   = 23
   Email = jane.doe@somedomain.com
   Phone = 0123456789 (HOME)
   Phone = 0612345678 (MOBILE)
   Phone = splicer (UNDEFINED)
   Friend:
      Name  = John Doe
      Age   = 35
      Email = john.doe@somedomain.com



.. |caution| image:: ./images/icon-caution.*
            :height: 6mm
.. |info|   image:: ./images/icon-info.*
            :height: 6mm
.. |windows| image:: ./images/icon-windows.*
            :height: 6mm
.. |unix| image:: ./images/icon-unix.*
            :height: 6mm
.. |linux| image:: ./images/icon-linux.*
            :height: 6mm
.. |c| image:: ./images/icon-c.*
            :height: 6mm
.. |cpp| image:: ./images/icon-cpp.*
            :height: 6mm
.. |csharp| image:: ./images/icon-csharp.*
            :height: 6mm
.. |java| image:: ./images/icon-java.*
            :height: 6mm

