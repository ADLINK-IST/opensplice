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
#include "idl.h"
#include "idl_extern.h"
#include "idl_bool.h"
#include "xbe_globals.h"
#include "xbe_utils.h"

extern void DDSError(const DDS_StdString &message);

static void
DDSStripArg(int& argc, char** argv, int index)
{
   int t;
   char * endholder = argv[index];

   for (t = index; t < argc - 1; t++)
   {
      argv[t] = argv[t + 1];
   }

   argv[argc - 1] = endholder;    // Not really necessary if argc decremented
   argc--;
}

void BE_prep_arg (char * arg, idl_bool)
{}

void BE_param_copy (char * trg, int argc, char **argv)
{}

void BE_usage()
{
   cerr << GTDEVEL(" -version\t\tdisplay version information\n");
   //cerr << GTDEVEL(" -client_only\t\tgenerate client implementation only\n");
   cerr << GTDEVEL(" -output=<dir>\t\tgenerate files into directory <dir>\n");
   cerr << GTDEVEL(" -import_export=<macro>[,<header_file>] defines dll macro name and optionally a header file which contains the macro\n");
   cerr << GTDEVEL(" -isocpp\t\tEnables ISO C++ API compilation\n");
   cerr << GTDEVEL(" -iso\t\t\tEnables ISO C++/C++11 types support\n");
   cerr << GTDEVEL(" -cext=<ext>\t\tuse <ext> for implementation files\n");
   cerr << GTDEVEL(" -hext=<ext>\t\tuse <ext> for header files\n");
   cerr << GTDEVEL(" -ch=<filename>\t\tset client header filename to <filename>\n");
   cerr << GTDEVEL(" -ci=<filename>\t\tset client implementation filename to <filename>\n");
   cerr << GTDEVEL(" -sh=<filename>\t\tset server header filename to <filename>\n");
   cerr << GTDEVEL(" -si=<filename>\t\tset server implementation filename to <filename>\n");
   cerr << GTDEVEL(" -th=<filename>\t\tset tie header filename to <filename>\n");
   cerr << GTDEVEL(" -max_char=<num>\tset maximum number of characters per line\n");
   //  cerr << GTDEVEL(" -portable_exceptions\tturn on portable exceptions support\n");
   //  cerr << GTDEVEL(" -per_request_attrs\tturn on per-request attributes support\n");
   cerr << GTDEVEL(" -gen_externalization\tgenerate code for externalization support\n");
   cerr << GTDEVEL(" -gen_onefile\t\tgenerate code to one file\n");
   cerr << GTDEVEL(" -map_wide\t\tconvert wide char and string to non-wide\n");
   cerr << GTDEVEL(" -case\t\t\tIdentifiers are case sensitive\n");
   cerr << GTDEVEL(" -ignore_interfaces\tDo not generate interface code\n");
   //cerr << GTDEVEL(" -no_warn\t\tDisable warning messages\n");
   cerr << GTDEVEL(" -collocated_direct\tGenerate code for direct servant invocation\n");
   cerr << GTDEVEL(" -[no]exceptions\tGenerate code for native/non native exceptions\n");
   cerr << GTDEVEL(" -isotest\t\t\tProduces test code, == and fill methods\n");
   cerr << GTDEVEL(" -genequality\t\t\tProduces equality overload methods\n");
}

void
DDS_BE_parse_args(int &argc, char **argv)
{
   // PARSE COMMAND LINE
   BE_Globals::client_only = pbtrue;
   BE_Globals::isocpp_new_types = pbfalse;
   idl_global->set_warn(I_FALSE);

   for (int i = 0; i < argc; i++)
   {
      if (strcmp(argv[i], "-version") == 0)
      {
         BE_version();
         exit(0);
      }
      else if (strncmp(argv[i], "-import_export",
                        sizeof ("-import_export") - 1) == 0)
      {
        char* macroName;
        char* macroHeader = NULL;

        /* Locate the macro name by searching for the '=' character. */
        macroName = strchr(argv[i], '=');
        /* Only continue if we found the '=' character */
        if(macroName)
        {
            /* The macro name starts after the '=' character, so move the
             * pointer. Do not do it before, because if the '=' character is not
             * found it would lead to a crash
             */
            macroName = macroName + 1;
            /* Now search for the ',' character. If found it means a header file
             * containing the macro was also defined.
             */
            macroHeader = strchr(macroName, ',');
            if(macroHeader)
            {
                /*If we found the ',' character, then change the character to a
                 * string terminator so that the macro name is cut off at the
                 * ',' character, and then move the pointer by 1 to find the start
                 * of the header file
                 */
                *macroHeader = '\0';
                macroHeader = macroHeader + 1;
            }
        }
        BE_Globals::UserDLL = macroName;
        BE_Globals::UserDLLHeader = macroHeader ? macroHeader : "";
        DDSStripArg(argc, argv, i);
        i = 0;
      }
      else if (strcmp(argv[i], "-ignore_interfaces") == 0)
      {
         BE_Globals::ignore_interfaces = pbtrue;
         idl_global->set_ignore_interfaces(I_TRUE);
         DDSStripArg(argc, argv, i);
         i = 0;
      }
      else if (strcmp(argv[i], "-isocpp") == 0)
      {
         BE_Globals::isocpp = pbtrue;
         DDSStripArg(argc, argv, i);
         i = 0;
      }
      else if (strcmp(argv[i], "-iso") == 0)
      {
         /* If the new types are enabled should we produce additional test code
          * such as the equals and fill operators*
          */
         for(int j=0; j < argc; j++)
         {
             if(0 == strcmp(argv[j], "-isotest"))
             {
                 BE_Globals::isocpp_test_methods = pbtrue;
                 BE_Globals::gen_equality = pbtrue;
                 DDSStripArg(argc, argv, j);
                 j=0;
             }
         }
         BE_Globals::isocpp_new_types = pbtrue;
         DDSStripArg(argc, argv, i);
         i = 0;
      }
      else if (0 == strcmp(argv[i], "-genequality"))
      {
         BE_Globals::gen_equality = pbtrue;
         DDSStripArg(argc, argv, i);
         i = 0;
      }
      else if (strncmp(argv[i], "-output=", sizeof("-output=") -1) == 0)
      {
         BE_Globals::OutputDirectory = strchr(argv[i], '=') + 1;
         DDSStripArg(argc, argv, i);
         i = 0;
      }
      else if (strncmp(argv[i], "-ch=", sizeof("-ch=") - 1) == 0)
      {
         BE_Globals::ClientHeaderFilename = strchr(argv[i], '=') + 1;
         DDSStripArg(argc, argv, i);
         i = 0;
      }
      else if (strncmp(argv[i], "-ci=", sizeof("-ci=") - 1) == 0)
      {
         BE_Globals::ClientImplFilename = strchr(argv[i], '=') + 1;
         DDSStripArg(argc, argv, i);
         i = 0;
      }
      else if (strncmp(argv[i], "-sh=", sizeof("-sh=") - 1) == 0)
      {
         BE_Globals::ServerHeaderFilename = strchr(argv[i], '=') + 1;
         DDSStripArg(argc, argv, i);
         i = 0;
      }
      else if (strncmp(argv[i], "-si=", sizeof("-si=") - 1) == 0)
      {
         BE_Globals::ServerImplFilename = strchr(argv[i], '=') + 1;
         DDSStripArg(argc, argv, i);
         i = 0;
      }
      else if (strncmp(argv[i], "-th=", sizeof("-th=") - 1) == 0)
      {
         BE_Globals::TieHeaderFilename = strchr(argv[i], '=') + 1;
         DDSStripArg(argc, argv, i);
         i = 0;
      }
      else if (strncmp(argv[i], "-cext=", sizeof("-cext=") - 1) == 0)
      {
         BE_Globals::CExtension = strchr(argv[i], '=') + 1;
         DDSStripArg(argc, argv, i);
         i = 0;
      }
      else if (strncmp(argv[i], "-hext=", sizeof("-hext=") - 1) == 0)
      {
         BE_Globals::hExtension = strchr(argv[i], '=') + 1;
         DDSStripArg(argc, argv, i);
         i = 0;
      }
      else if (strcmp(argv[i], "-gen_gui_info") == 0)
      {
         BE_Globals::gen_gui_info = pbtrue;
         DDSStripArg(argc, argv, i);
         i = 0;
      }
//       else if (strcmp(argv[i], "-client_only") == 0)
//       {
//          BE_Globals::client_only = pbtrue;
//          DDSStripArg(argc, argv, i);
//          i = 0;
//       }
      else if (strncmp(argv[i], "-max_char=", sizeof("-max_char=") - 1) == 0)
      {
         BE_Globals::max_char_per_line = atoi(strchr(argv[i], '=') + 1);
         DDSStripArg(argc, argv, i);
         i = 0;
      }
//       else if (strcmp(argv[i], "-no_warn") == 0)
//       {
//          idl_global->set_warn(I_FALSE);
//          DDSStripArg(argc, argv, i);
//          i = 0;
//       }
      else if (strcmp(argv[i], "-portable_exceptions") == 0)
      {
         BE_Globals::portable_exceptions = pbtrue;
         DDSStripArg(argc, argv, i);
         i = 0;
      }
      else if (strcmp(argv[i], "-per_request_attrs") == 0)
      {
         BE_Globals::per_request_attrs = pbtrue;
         DDSStripArg(argc, argv, i);
         i = 0;
      }
      else if (strcmp(argv[i], "-gen_externalization") == 0)
      {
         BE_Globals::gen_externalization = pbtrue;
         DDSStripArg(argc, argv, i);
         i = 0;
      }
//       else if (strcmp(argv[i], "-exceptions") == 0)
//       {
//          XBE_Ev::_mode = XBE_Ev::XBE_ENV_EXCEPTION;
//          DDSStripArg (argc, argv, i);
//          i = 0;
//       }
//       else if (strcmp(argv[i], "-noexceptions") == 0)
//       {
//          XBE_Ev::_mode = XBE_Ev::XBE_ENV_NO_EXCEPTION;
//          DDSStripArg(argc, argv, i);
//          i = 0;
//       }
      else if (strcmp(argv[i], "-gen_onefile") == 0)
      {
         BE_Globals::gen_onefile = pbtrue;
         DDSStripArg(argc, argv, i);
         i = 0;
      }
      else if (strcmp(argv[i], "-map_wide") == 0)
      {
         BE_Globals::map_wide = pbtrue;
         DDSStripArg(argc, argv, i);
         i = 0;
      }
      else if (strcmp(argv[i], "-case") == 0)
      {
         BE_Globals::case_sensitive = pbtrue;
         DDSStripArg(argc, argv, i);
         i = 0;
      }
      else if (strcmp(argv[i], "-collocated_direct") == 0)
      {
         BE_Globals::collocated_direct = pbtrue;
         DDSStripArg(argc, argv, i);
         i = 0;
      }
   }

   if (BE_Globals::client_only == pbtrue)
   {
      // direct collocation is generated server side
      BE_Globals::collocated_direct = pbfalse;
   }
}
