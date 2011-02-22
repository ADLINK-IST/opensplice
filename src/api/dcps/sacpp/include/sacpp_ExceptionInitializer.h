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
#ifndef SACPP_EXCEPTIONINITIALIZER_H
#define SACPP_EXCEPTIONINITIALIZER_H

#include "sacpp_DDS_DCPS.h"
#include "sacpp_Exception.h"
#include "sacpp_if.h"

/**
 * @brief Holds statically registered information for each concrete
 * DDS::Exception subtype, and provides a factory facility based on repository
 * ids.
 *
 * Each concrete DDS::Exception subtype should create a static instance of
 * this class. Each of these instances should be initialized with the repository
 * id and the factory function for that specific DDS::Exception subtype.
 *
 * Also, during construction, each instance hooks itself into a global single
 * linked list (m_head).  Therefore, when static initialization is complete, we
 * have a list containing the repository id and factory function for all
 * DDS::Exception subtypes that are in the image.
 *
 * This list is then used by the create() method which provides the ability to
 * create new DDS::Exception objects of the appropriate subtype, given a
 * repository id.
 */
class SACPP_API DDS::ExceptionInitializer
{
   public:

      typedef DDS::Exception* (*Factory)();

   public:

      ExceptionInitializer (const char *name, Factory factory);

      static Factory lookup (const char *name);

   private:

      // Assignment not permitted
      ExceptionInitializer & operator=( const ExceptionInitializer & );

      static ExceptionInitializer *m_head;
      ExceptionInitializer        *m_next;

      const char *  m_name;
      const Factory m_fact;
};

#undef SACPP_API
#endif
