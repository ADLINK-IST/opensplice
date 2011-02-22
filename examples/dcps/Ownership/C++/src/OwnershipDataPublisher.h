
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
      OwnershipDataPublisher(string pub, CORBA::Long strength);

      void setStrength(CORBA::Long strength);

      void publishEvent(float price, string pub);

      void dispose();

    private:
      DDSEntityManager *mgr;
      DataWriter_ptr dWriter;
      StockDataWriter_var OwnershipDataDataWriter;
      InstanceHandle_t userHandle;
      Stock *m_instance;
      ReturnCode_t status;
      void initPublisher(string pub, CORBA::Long strength);
  };
#endif
