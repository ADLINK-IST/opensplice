
/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

/************************************************************************
 * LOGICAL_NAME:    OwnershipDataPublisher.h
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C++ programming language.
 * DATE             September 2010.
 ************************************************************************
 *
 * This file contains the implementation for the 'OwnershipDataPublisher' executable.
 *
 ***/
#ifndef STOCKQUOTEPUB_H
  #define STOCKQUOTEPUB_H
  #include <string>
  #include <sstream>
  #include <iostream>
  #include "DDSEntityManager.h"
  #include "ccpp_OwnershipData.h"

  using namespace DDS;
  using namespace OwnershipData;

  class OwnershipDataPublisher
  {

    public:
      OwnershipDataPublisher(string pub, DDS::Long strength);

      void publishEvent(float price, string pub);

      void dispose();

    private:
      DDSEntityManager mgr;
      DataWriter_var dWriter;
      StockDataWriter_var OwnershipDataDataWriter;
      InstanceHandle_t userHandle;
      Stock *m_instance;
      ReturnCode_t status;
      void initPublisher(string pub, DDS::Long strength);
  };
#endif
