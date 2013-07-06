/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef _UTL_INCL_HH_
#define _UTL_INCL_HH_

class UTL_IncludeFiles
{

public:
   
   virtual ~UTL_IncludeFiles()
   {}

   static void AddIncludeFile(char *header, char *path)
   {
      if (realImplementation)
         realImplementation->reallyAddIncludeFile(header, path);
   }

protected:
   virtual void reallyAddIncludeFile(char *header, char *path) = 0;

private:
   static UTL_IncludeFiles *realImplementation;
};

#endif
