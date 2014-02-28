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
#ifdef SCCSID
static char SCCSid[] = "%W% %G%";
#endif

#include "idl.h"
#include "idl_extern.h"
#include "xbe_globals.h"
#include "xbe_generator.h"
#include "xbe_root.h"
#include "xbe_source.h"
#include "xbe_utils.h"
#include "cppgen_iostream.h"

/*
 * Do the code generation
 */
void BE_produce ()
{
   be_root * beRoot;

   assert(idl_global);
   assert(idl_global->main_filename());
   assert(idl_global->root());

   if (idl_global && idl_global->root() &&
         (beRoot = (be_root*)idl_global->root()->narrow((long) & be_root::type_id)))
   {

      be_Source::Initialize();
      BE_Globals::Initialize();

      // GENERATE BASE.H, BASE.C

      if (char* mainFilename = idl_global->main_filename()->get_string())
      {
         DDS_StdString BaseFilename;
         DDS_StdString output_dir;

         BaseFilename = StripExtension(FindFilename(mainFilename));

         // DAH Fix
         // On linux the commented out line resolved ../hello.idl
         // to hello.  On Win32 it resolved it to ../hello
         // The replacement line fixes the Win32 problem.

         //BaseFilename = FilterFilename(BaseFilename);
         BaseFilename = BaseName(BaseFilename);

         unsigned int len = BE_Globals::OutputDirectory.length ();
         if (len > 0)
         {
            if ((BE_Globals::OutputDirectory[len - 1] == '/'))
            {
               output_dir = BE_Globals::OutputDirectory;
            }
#if defined(_WIN32)
            else if ((BE_Globals::OutputDirectory[len - 1] == '\\'))
            {
               output_dir = BE_Globals::OutputDirectory;
            }
            else
            {
               output_dir = BE_Globals::OutputDirectory + "\\";
            }
#else
            else
            {
               output_dir = BE_Globals::OutputDirectory + "/";
            }
#endif

        }

         // these if's make sure that we don't override filenames
         // specified on the idlc command line
       if (BE_Globals::gen_onefile == TRUE)
       {
          BE_Globals::BaseFilename = BaseFilename;

          BE_Globals::ClientHeaderFilename = BaseFilename + "." + BE_Globals::hExtension;

          BE_Globals::ClientImplFilename = BaseFilename + "." + BE_Globals::CExtension;

          BE_Globals::ServerHeaderFilename = BaseFilename + "." + BE_Globals::hExtension;

          BE_Globals::ServerImplFilename = BaseFilename + "." + BE_Globals::CExtension;

          BE_Globals::TieHeaderFilename = BaseFilename + "." + BE_Globals::hExtension;
       }
       else
       {

         if (BE_Globals::gen_onefile == TRUE)
         {
            BE_Globals::BaseFilename = BaseFilename;

            BE_Globals::ClientHeaderFilename = BaseFilename + "." + BE_Globals::hExtension;

            BE_Globals::ClientImplFilename = BaseFilename + "." + BE_Globals::CExtension;

            BE_Globals::ServerHeaderFilename = BaseFilename + "." + BE_Globals::hExtension;

            BE_Globals::ServerImplFilename = BaseFilename + "." + BE_Globals::CExtension;

            BE_Globals::TieHeaderFilename = BaseFilename + "." + BE_Globals::hExtension;
         }
         else
         {

            if ((char*)BE_Globals::ClientHeaderFilename == NULL)
            {
               BE_Globals::ClientHeaderFilename = BaseFilename +
                                               BE_Globals::ClientExtension + "." + BE_Globals::hExtension;
            }

            if ((char*)BE_Globals::ClientImplFilename == NULL)
            {
               BE_Globals::ClientImplFilename = BaseFilename +
                                             BE_Globals::ClientExtension + "." + BE_Globals::CExtension;
            }

            if ((char*)BE_Globals::ServerHeaderFilename == NULL)
            {
               BE_Globals::ServerHeaderFilename = BaseFilename +
                                               BE_Globals::ServerExtension + "." + BE_Globals::hExtension;
            }

            if ((char*)BE_Globals::ServerImplFilename == NULL)
            {
               BE_Globals::ServerImplFilename = BaseFilename +
                                             BE_Globals::ServerExtension + "." + BE_Globals::CExtension;
            }

            if ((char*)BE_Globals::TieHeaderFilename == NULL)
            {
               BE_Globals::TieHeaderFilename = BaseFilename +
                                            BE_Globals::TieExtension + "." + BE_Globals::hExtension;
            }
        }

       }
         // CLIENT HEADER
         be_ClientHeader clientHeader;
         if (clientHeader.Open(output_dir + BE_Globals::ClientHeaderFilename))
         {
            beRoot->Generate(clientHeader);

            if (BE_Globals::gen_gui_info)
            {
               cerr << "generated:" << BE_Globals::ClientHeaderFilename << nl;
            }

            // CLIENT IMPLEMENTATION
            beRoot->FinishAllProtoTypeCodes();

            be_ClientImplementation clientImplementation;

            if (clientImplementation.Open(output_dir + BE_Globals::ClientImplFilename))
            {
               beRoot->Generate(clientImplementation);

               if (BE_Globals::gen_gui_info)
               {
                  cerr << "generated:" << (const char*)BE_Globals::ClientImplFilename << nl;
               }

               // SERVER HEADER
               if (!BE_Globals::client_only)
               {
                  be_ServerHeader serverHeader;

                  if (serverHeader.Open(output_dir + BE_Globals::ServerHeaderFilename))
                  {
                     beRoot->Generate(serverHeader);

                     if (BE_Globals::gen_gui_info)
                     {
                        cerr << "generated:" << (const char*)BE_Globals::ServerHeaderFilename << nl;
                     }

                     // SERVER IMPLEMENTATION
                     be_ServerImplementation serverImplementation;

                     if (serverImplementation.Open(output_dir + BE_Globals::ServerImplFilename))
                     {
                        beRoot->Generate(serverImplementation);

                        if (BE_Globals::gen_gui_info)
                        {
                           cerr << "generated:" << (const char*)BE_Globals::ServerImplFilename << nl;
                        }
                     }
                     else
                     {
                        DDSError((DDS_StdString) "unable to open file: " + BE_Globals::ServerImplFilename);
                     }
                  }
                  else
                  {
                     DDSError((DDS_StdString) "unable to open file: " + BE_Globals::ServerHeaderFilename);
                  }
               }
            }
            else
            {
               DDSError((DDS_StdString) "unable to open file: " + BE_Globals::ClientImplFilename);
            }
         }
         else
         {
            DDSError((DDS_StdString) "unable to open file: " + BE_Globals::ClientHeaderFilename);
         }
      }
      else
      {
         DDSError("internal: backend has no main filename");
      }
   }
   else
   {
      DDSError("internal: backend has no AST root");
   }
}

/*
 * Abort this run of the BE
 */
void BE_abort()
{}

/*
 * delete all the output files; ignore errors
 *
 * On Windows, must be called after the files are closed.
 */
void BE_unlinkAllFiles()
{
   DDS_StdString* allFiles[] = {
                                  &BE_Globals::ClientHeaderFilename,
                                  &BE_Globals::ClientImplFilename,
                                  &BE_Globals::ServerHeaderFilename,
                                  &BE_Globals::ServerImplFilename,
                                  NULL
                               };

   for (DDS_StdString** pfile = allFiles; *pfile; pfile++)
   {
      unlink((const char *)**pfile);
   }
}
