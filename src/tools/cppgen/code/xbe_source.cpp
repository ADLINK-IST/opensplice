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
#include "os_version.h"
#include "idl.h"
#include <ctype.h>
#include "idl_extern.h"
#include "xbe_source.h"
#include "xbe.h"
#include "xbe_incl.h"
#include "Std.h"
#include "cppgen_iostream.h"
#include "xbe_time.h"
#ifdef __APPLE__
#include <sys/time.h>
#endif

// ----------------------------------------------------------
//  DDSSource_Time IMPLEMENTATION
// ----------------------------------------------------------

const unsigned long secsInUsec = 1000000;
const unsigned long secsInNsec = 1000000000;
const unsigned int secsInMin = 60;

class DDSSource_Time
{
public:

   DDSSource_Time();
   DDSSource_Time(unsigned long secs, unsigned long usec);
   DDSSource_Time(unsigned long timeValue);
#ifdef __APPLE__
   DDSSource_Time(const timeval &nativeTimeValue);
#else
   DDSSource_Time(const timespec &nativeTimeValue);
#endif

   ~DDSSource_Time();

   operator char*() const;

   static DDSSource_Time get_time_of_day();

private:

   unsigned long m_secs;
   unsigned long m_usec;
};

DDSSource_Time::DDSSource_Time()
      :
      m_secs(0),
      m_usec(0)
{}

DDSSource_Time::DDSSource_Time(unsigned long secs, unsigned long usec)
      :
      m_secs(secs),
      m_usec(usec)
{}

DDSSource_Time::DDSSource_Time(unsigned long timeValue)
{
   m_secs = (unsigned long) (timeValue / secsInUsec);
   m_usec = (unsigned long) (timeValue % secsInUsec);
}

#ifdef __APPLE__
DDSSource_Time::DDSSource_Time(const timeval &timeValue)
{
   m_secs = timeValue.tv_sec + (timeValue.tv_usec / secsInUsec);
   m_usec = timeValue.tv_usec;
}
#else
DDSSource_Time::DDSSource_Time(const timespec &timeValue)
{
   m_secs = timeValue.tv_sec + (timeValue.tv_nsec / secsInNsec);
   m_usec = (timeValue.tv_nsec % secsInNsec) /1000;
}
#endif

DDSSource_Time::~DDSSource_Time()
{}



DDSSource_Time::operator char* () const
{
   time_t clock = (time_t) m_secs;
   char * ascTime = ctime (&clock);

   if (ascTime)
   {
      // KILL THE TRAILING NEW-LINE
      ascTime[strlen(ascTime) - 1] = '\0';
   }

   return ascTime;
}


DDSSource_Time
DDSSource_Time::get_time_of_day()
{
#if !defined(_WIN32)
#ifdef __APPLE__
   timeval tval;
   gettimeofday(&tval, NULL);
#else
   timespec tval;
   clock_gettime(CLOCK_REALTIME, &tval);
#endif /* __APPLE__ */
   return DDSSource_Time(tval);
#else

   SYSTEMTIME sys_time;
   FILETIME file_time;
   timeval tmp;

   GetSystemTime(&sys_time);

   if (!SystemTimeToFileTime(&sys_time, &file_time))
   {
      return DDSSource_Time();
   }

   // Calculate microseconds component of current time using system time.
   tmp.tv_usec = file_time.dwLowDateTime / 10;  // covert to microseconds

   while (tmp.tv_usec >= 1000000) //make sure it less than 1,000,000
   {
      tmp.tv_usec -= 1000000;
   }
   // Set seconds component of current time: number of seconds
   // since midnight, January 1, 1970

   tmp.tv_sec = time(NULL);

   return DDSSource_Time(tmp.tv_sec, tmp.tv_usec);

#endif
}

// ----------------------------------------------------------
//  XBE_SOURCE IMPLEMENTATION
// ----------------------------------------------------------
const DDS_StdString be_Source::PublicAccess = "public:";
const DDS_StdString be_Source::ProtectedAccess = "protected:";
const DDS_StdString be_Source::PrivateAccess = "private:";

String_map be_Source::idlIncludes(idlc_hash_str);
String_map be_Source::otherIncludes(idlc_hash_str);

be_Source::be_Source ()
   : margin (""), closed (FALSE)
{}

void be_Source::Initialize ()
{
   UTL_String **includes = idl_global->include_file_names();
   DDS_StdString includeFile;

   for (unsigned int i = 0; i < idl_global->n_include_file_names(); i++)
   {
      if (strstr(includes[i]->get_string(), ".IDL") || strstr(includes[i]->get_string(), ".idl"))
      {
         includeFile = StripExtension(includes[i]->get_string());
         includeFile = FilterFilename(includeFile);

         if (includeFile != "orb")
         {
            idlIncludes[includeFile] = includeFile;
         }
      }
      else
      {
         DDS_StdString oi = includes[i]->get_string();
         otherIncludes[oi] = oi;
      }
   }
}

pbbool be_Source::Open (const DDS_StdString & mainFilename)
{
   pbbool ret;
   DDSSource_Time cTime = DDSSource_Time::get_time_of_day ();

   m_filename = mainFilename;
   creationTime = (char*)cTime;

   /** @bug OSPL-3472 will append regenerated files
    * proposed fix - Add find() on DDS_StdString **/
   if (BE_Globals::isocpp_test_methods)
       os.open((const char *)m_filename, ios_base::app);
   else
       os.open((const char *)m_filename, ios_base::out);

   ret = (pbbool) (os.rdbuf() && os.rdbuf()->is_open());
   closed = !ret;

   return ret;
}

void be_Source::Close ()
{
   closed = TRUE;
   os.close ();
}

void be_Source::Outdent ()
{
   if (margin.length())
   {
      DDS_StdString t = margin;

      ((char *)t)[margin.length() - 3] = (char)0;
      margin = t;
   }
}

void be_Source::SetAccess (const DDS_StdString & access)
{
   be_Tab tab(*this);

   Outdent();
   Stream() << tab << access << nl;
   Indent();
}

be_Source::~be_Source ()
{
   if (! closed)
   {
      os.close ();
   }
}

// ----------------------------------------------------------
//  STUB DEFINITION
// ----------------------------------------------------------
be_ClientHeader::be_ClientHeader()
{}


pbbool be_ClientHeader::Open (const DDS_StdString& mainFilename)
{
   if (be_Source::Open (mainFilename))
   {
      BE_Globals::HFileOpen = pbtrue;
      os << "//******************************************************************\n"
         << "// \n"
         << "//  Generated by IDL to C++ Translator\n"
         << "//  \n"
         << "//  File name: " << BE_Globals::ClientHeaderFilename << "\n"
         << "//  Source: " << idl_global->main_filename()->get_string() << "\n"
         << "//  Generated: " << CreationTime() << "\n"
         << "//  OpenSplice " << OSPL_VERSION_STR << "\n"
         << "//  \n"
         << "//******************************************************************\n";

      ndefname = Ifndefize(BE_Globals::ClientHeaderFilename);
      os << "#ifndef " << (const char*) ndefname << nl;
      os << "#define " << (const char*) ndefname << nl;
      if (BE_Globals::isocpp || BE_Globals::isocpp_new_types)
      {
        os << "#ifndef OPENSPLICE_ISOCXX_PSM" << nl;
        os << "#define OPENSPLICE_ISOCXX_PSM" << nl;
        os << "#endif" << nl;
      }
      os << nl << "#include \"sacpp_mapping.h\"" << nl;
      os << "#include \"sacpp_DDS_DCPS.h\"" << nl;
      if (BE_Globals::isocpp_new_types)
      {
        os << "#include <dds/core/ddscore.hpp>" << nl;
      }
      if (BE_Globals::isocpp_test_methods)
      {
        os << "#include <generate_test_values.hpp>" << nl;
      }
      if (BE_Globals::UserDLL != (const char *)"" &&
          BE_Globals::UserDLLHeader != (const char *)"" )
      {
        os << "#include \"" << BE_Globals::UserDLLHeader << "\"" << nl;
      }

      GenerateSecondaryIncludes (os);

      if (BE_Globals::UserDLL != (const char *)"")
      {
         BE_Globals::DLLExtension = " ";
      }
      os << nl;
      return pbtrue;
   }
   else
   {
      return pbfalse;
   }
}


void
be_ClientHeader::GenerateSecondaryIncludes(ostream& hos)
{
   // Include other idl bases

   String_map::iterator it;

   for (it = idlIncludes.begin() ; it != idlIncludes.end(); it++)
   {
      DDS_StdString file(it.value() + BE_Globals::ClientExtension + "." + BE_Globals::hExtension);

      if (DDSRealIncludeFiles::includes[file].length() == 0)
      {
         hos << "#include \"" << (const char*)file << "\"" << nl;
      }
      else
      {
         hos << "#include \"" << (const char*)DDSRealIncludeFiles::includes[file] << "\"" << nl;
      }
   }

   // Include any other includes

   for (it = otherIncludes.begin() ; it != otherIncludes.end(); it++)
   {
      /*
       * Check not preproccessor special include. When preprocessing with
       * gcc -E using gcc version 3 or above the preprocessed file also includes
       * <command line> and <built-in>. These are not real includes and so need
       * filtering out here.
       */

      const char* val = (const char*)it.value ();
      if (*val != '<')
      {
         hos << "#include \"" << val << "\"" << nl;
      }
   }
}


be_ClientHeader::~be_ClientHeader()
{}


// ----------------------------------------------------------
//  STUB IMPLEMENTATION
// ----------------------------------------------------------
be_ClientImplementation::be_ClientImplementation()
{}


pbbool be_ClientImplementation::Open(const DDS_StdString& mainFilename)
{
   if (be_Source::Open (mainFilename))
   {
      BE_Globals::CFileOpen = pbtrue;
      os << "//******************************************************************\n"
         << "// \n"
         << "//  Generated by IDL to C++ Translator\n"
         << "//  \n"
         << "//  File name: " << BE_Globals::ClientImplFilename << "\n"
         << "//  Source: " << idl_global->main_filename()->get_string() << "\n"
         << "//  Generated: " << CreationTime() << "\n"
         << "//  OpenSplice " << OSPL_VERSION_STR << "\n"
         << "//  \n"
         << "//******************************************************************\n";
      os << nl;

      if (DDSRealIncludeFiles::includes[FindFilename(BE_Globals::ClientHeaderFilename)].length() == 0)
      {
         if (BE_Globals::collocated_direct)
         {
            os << "#include \""
               << (const char*)BE_Globals::ServerHeaderFilename
               << "\"" << nl;
         }
         else
         {
            os << "#include \""
               << (const char*)BE_Globals::ClientHeaderFilename
               << "\"" << nl;
         }
      }
      else
      {
         if (BE_Globals::collocated_direct)
         {
            os << "#include \""
               << (const char*)DDSRealIncludeFiles::includes[FindFilename(BE_Globals::ServerHeaderFilename)]
               << "\"" << nl;
         }
         else
         {
            os << "#include \""
               << (const char*)DDSRealIncludeFiles::includes[FindFilename(BE_Globals::ClientHeaderFilename)]
               << "\""
               << nl;
         }
      }

      os << nl;

      return pbtrue;
   }
   else
   {
      return pbfalse;
   }
}

be_ClientImplementation::~be_ClientImplementation()
{}

// ----------------------------------------------------------
//  IMPL DEFINITION
// ----------------------------------------------------------

be_ServerHeader::be_ServerHeader () {}
be_ServerHeader::~be_ServerHeader () {}

pbbool be_ServerHeader::Open(const DDS_StdString& mainFilename)
{
   if (be_Source::Open(mainFilename))
   {
      if (!BE_Globals::gen_onefile)
      {

         os << "//******************************************************************\n"
         << "// \n"
         << "//  Generated by IDL to C++ Translator\n"
         << "//  \n"
         << "//  File name: " << BE_Globals::ServerHeaderFilename << "\n"
         << "//  Source: " << idl_global->main_filename()->get_string() << "\n"
         << "//  Generated: " << CreationTime() << "\n"
         << "//  OpenFusion V" << OSPL_VERSION_STR << "\n"
         << "//  \n"
         << "//******************************************************************\n";
      }

      if (BE_Globals::gen_onefile)
      {
         ndefname = Ifndefize(BE_Globals::ServerHeaderFilename);
      }
      else
      {
         ndefname = Ifndefize(BE_Globals::ServerHeaderFilename);
      }

      os << "#ifndef " << (const char*)ndefname << nl;
      os << "#define " << (const char*)ndefname << nl << nl;
      os << nl;

      os << "#include \"eOrb/idl_s.h\"" << nl;

      if (DDSRealIncludeFiles::includes[FindFilename(BE_Globals::ClientHeaderFilename)].length() == 0)
      {
         if (!BE_Globals::gen_onefile)
         {
            os << "#include \"" << (const char*)BE_Globals::ClientHeaderFilename << "\"" << nl;
         }
      }
      else
      {
         if (!BE_Globals::gen_onefile)
         {
            os << "#include \"" << (const char*)DDSRealIncludeFiles::includes[FindFilename(BE_Globals::ClientHeaderFilename)] << "\"" << nl;
         }
      }

      String_map::iterator it;

      for (it = idlIncludes.begin() ; it != idlIncludes.end(); it++)
      {
                 DDS_StdString file(it.value() + BE_Globals::ServerExtension + "." + BE_Globals::hExtension);

         if (DDSRealIncludeFiles::includes[file].length() == 0)
         {
            os << "#include \"" << (const char*)file << "\"" << nl;
         }
         else
         {
            os << "#include \"" << (const char*)DDSRealIncludeFiles::includes[file] << "\"" << nl;
         }
      }

      os << nl;

      return pbtrue;
   }
   else
   {
      return pbfalse;
   }
}

be_ServerImplementation::be_ServerImplementation () {}
be_ServerImplementation::~be_ServerImplementation () {}

pbbool be_ServerImplementation::Open (const DDS_StdString & mainFilename)
{
   if (be_Source::Open (mainFilename))
   {
      os << "//******************************************************************\n"
      << "// \n"
      << "//  Generated by IDL to C++ Translator\n"
      << "//  \n"
      << "//  File name: " << BE_Globals::ServerImplFilename << "\n"
      << "//  Source: " << idl_global->main_filename()->get_string() << "\n"
      << "//  Generated: " << CreationTime() << "\n"
      << "//  OpenFusion V" << OSPL_VERSION_STR << "\n"
      << "//  \n"
      << "//******************************************************************\n";

      if (DDSRealIncludeFiles::includes[FindFilename(BE_Globals::ServerHeaderFilename)].length() == 0)
      {
         if (!BE_Globals::gen_onefile)
         {
            os << "#include \"" << (const char*)BE_Globals::ServerHeaderFilename << "\"" << nl;
         }
      }
      else
      {
         if (!BE_Globals::gen_onefile)
         {
            os << "#include \"" << (const char*)DDSRealIncludeFiles::includes[FindFilename(BE_Globals::ServerHeaderFilename)] << "\"" << nl;
         }
      }

      os << nl;

      return pbtrue;
   }
   else
   {
      return pbfalse;
   }
}

// ----------------------------------------------------------
//  TIE HEADER DEFINITION
// ----------------------------------------------------------
#if defined(DDS_TIE_HEADER)
be_ServerTieHeader::be_ServerTieHeader()
{}

pbbool
be_ServerTieHeader::Open(const DDS_StdString& mainFilename)
{
   if (be_Source::Open(mainFilename))
   {
      os << "//******************************************************************\n"
      << "// \n"
      << "//  Generated by IDL to C++ Translator\n"
      << "//  \n"
      << "//  File name: " << filename << "\n"
      << "//  Source: " << idl_global->main_filename()->get_string() << "\n"
      << "//  Generated: " << CreationTime() << "\n"
      << "//  OpenFusion V" << OSPL_VERSION_STR << "\n"
      << "//  \n"
      << "//******************************************************************\n";

      ndefname = Ifndefize(filename);

      os << "#ifndef " << (const char*)ndefname << nl;
      os << "#define " << (const char*)ndefname << nl << nl;
      os << nl;

      return pbtrue;
   }
}

be_ServerTieHeader::~be_ServerTieHeader()
{}

#endif
