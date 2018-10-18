/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef SACPP_EXCEPTIONINITIALIZER_H
#define SACPP_EXCEPTIONINITIALIZER_H

#include "sacpp_dds_basic_types.h"
#include "sacpp_Exception.h"
#include "cpp_dcps_if.h"

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
class OS_API DDS::ExceptionInitializer
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

#undef OS_API
#endif
