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
