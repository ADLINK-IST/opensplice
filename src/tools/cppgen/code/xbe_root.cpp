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

#include "xbe_utils.h"
#include "xbe_globals.h"
#include "xbe_literals.h"
#include "xbe_root.h"
#include "xbe_exception.h"
#include "xbe_interface.h"
#include "xbe_sequence.h"
#include "xbe_cppfwd.h"
#include "Std.h"
#include "xbe_genlist.h"

#include "cppgen_iostream.h"

static void
sanitize(DDS_StdString& str)
{
   for (unsigned int i = 0; i < str.length(); i++)
   {
      if ( (str[i] < 0x30) ||
            ( (str[i] > 0x39) && (str[i] < 0x41) ) ||
            ( (str[i] > 0x5A) && (str[i] < 0x61) && (str[i] != 0x5F) ) ||
            (str[i] > 0x7A) )
      {
         str[i] = '_';
      }
   }
}

static DDS_StdString
main_file_identifier()
{
   DDS_StdString mainFile = BaseName(idl_global->main_filename()->get_string());
   char *dot = (char*)strrchr((const char*)mainFile, '.');

   if (dot)
   {
      *dot = 0;
   }

   sanitize(mainFile);

   return mainFile;
}

// -------------------------------------------------
//  BE_ROOT IMPLEMENTATION
// -------------------------------------------------
IMPL_NARROW_METHODS2(be_root, AST_Root, be_CodeGenerator)
IMPL_NARROW_FROM_DECL(be_root)
IMPL_NARROW_FROM_SCOPE(be_root)

TList<be_Type *> &be_root::anyOps = *new TList<be_Type *>;
TList<be_Type *> &be_root::putGetOps = *new TList<be_Type *>;
TList<be_Type *> &be_root::streamOps = *new TList<be_Type *>;
TList<be_Type *> &be_root::typecodes = *new TList<be_Type *>;
TList<be_Type *> &be_root::typesThatNeedProtoTypeCodesFinished = *new TList<be_Type *>;
TList<be_Type *> &be_root::typedefs = *new TList<be_Type *>;
TList<be_Type *> &be_root::fwdDecls = *new TList<be_Type *>;
TList<be_exception *> &be_root::globalDecls = *new TList<be_exception *> ;
TList<be_CodeGenerator *> &be_root::implementations = *new TList<be_CodeGenerator *>;
TList<be_sequence*> &be_root::deferred_seqs = *new TList<be_sequence*>;

be_root::be_root ()
{
   isAtModuleScope (pbfalse);
}

be_root::be_root (UTL_ScopedName *n, const UTL_Pragmas &p)
   : AST_Root (n, p)
{
   isAtModuleScope (pbfalse);
}

void be_root::AddAnyOps (be_Type& btype)
{
   DDS::Boolean isUniqueOp = TRUE;
   TList<be_Type*>::iterator bit;

   //
   // multiple instantiations of an anonymous type can
   // cause duplicate definitions of an op without the
   // following filter
   //

   for (bit = anyOps.begin(); bit != anyOps.end(); bit++)
   {
      be_Type& thatType = *(*bit); // YO should be const

      if (btype.any_op_id() == thatType.any_op_id())
      {
         isUniqueOp = FALSE;
      }
   }

   if (isUniqueOp)
   {
      anyOps.push_back (&btype);
   }
}

void be_root::AddPutGetOps (be_Type& btype)
{
   DDS::Boolean isUniqueOp = TRUE;

   //
   // multiple instantiations of an anonymous type can
   // cause duplicate definitions of an op without the
   // following filter
   //

   if (isUniqueOp)
   {
      putGetOps.push_back (&btype);
   }
}

void be_root::AddStreamOps (be_Type& btype)
{
   DDS::Boolean isUniqueOp = TRUE;
   TList<be_Type *>::iterator bit;

   //
   // multiple instantiations of an anonymous type can
   // cause duplicate definitions of an op without the
   // following filter
   //

   for (bit = streamOps.begin(); bit != streamOps.end(); bit++)
   {
      if (btype.any_op_id() == (*bit)->any_op_id())
      {
         isUniqueOp = FALSE;
      }
   }

   if (isUniqueOp)
   {
      streamOps.push_back (&btype);
   }
}

void be_root::AddFwdDecls (be_Type & btype)
{
   fwdDecls.push_back (&btype);
}

void be_root::RemoveFwdDecls (be_Type & btype)
{
   fwdDecls.remove (&btype);
}

void be_root::AddTypedef (be_Type& btype)
{
   typedefs.push_back (&btype);
}

void be_root::AddTypecode (be_Type& btype)
{
   typecodes.push_back (&btype);
}

void be_root::AddTypeThatNeedsProtoTypeCodeFinished (be_Type& btype)
{
   typesThatNeedProtoTypeCodesFinished.push_back (&btype);
}

void be_root::AddGlobalDeclarations (be_exception * bexcept)
{
   assert (bexcept);
   globalDecls.push_back (bexcept);
}

void be_root::AddImplementations (be_CodeGenerator& cg)
{
   implementations.push_front (&cg);
}

void be_root::DeferSequence (be_sequence* s)
{
   assert (s);
   deferred_seqs.push_back (s);
}

void be_root::GenerateGlobalTypes (be_ClientHeader& source)
{
   UTL_ScopeActiveIterator * i;
   be_CodeGenerator * cg;
   AST_Decl * d;
   be_Type * bt;
   DDS_StdString scope;
   UTL_Scope * s = (UTL_Scope*) narrow ((long) & UTL_Scope::type_id);

   if (s)
   {
      // Iterate through types

      i = new UTL_ScopeActiveIterator (s, UTL_Scope::IK_localtypes);

      while (! i->is_done ())
      {
         d = i->item ();

         if (! d->imported ())
         {
            cg = (be_CodeGenerator*) d->narrow
               ((long) & be_CodeGenerator::type_id);
            if (cg)
            {
               bt = (be_Type*) d->narrow ((long) & be_Type::type_id);

               scope = bt->EnclosingScope ();
               if (! bt->HasTypeDef () && scope == NULL)
               {
                  cg->Generate (source);
               }
            }
         }

         i->next ();
      }

      delete i;
   }
}

void be_root::GenerateGlobalDecls (be_ClientHeader & source)
{
   UTL_ScopeActiveIterator * i;
   be_CodeGenerator * cg;
   AST_Decl * d;
   UTL_Scope * s = (UTL_Scope*) narrow ((long) & UTL_Scope::type_id);

   if (s)
   {
      // Iterate through decls

      i = new UTL_ScopeActiveIterator (s, UTL_Scope::IK_decls);

      while (!(i->is_done ()))
      {
         d = i->item ();

         if (!d->imported ())
         {
            cg = (be_CodeGenerator*) d->narrow
               ((long) & be_CodeGenerator::type_id);

            if (cg)
            {
               cg->Generate (source);
            }
         }

         i->next ();
      }

      delete i;
   }
}

void be_root::Generate (be_ClientHeader & source)
{
   be_CppFwdDecl::GenerateAllWithinScope (source, be_CppEnclosingScope());

   ostream& os = source.Stream ();
   TList<be_Type *>::iterator bit;
   TList<be_exception *>::iterator git;

   for (bit = fwdDecls.begin(); bit != fwdDecls.end(); bit++)
   {
      (*bit)->GenerateFwdDecls(source);
   }

   os << nl;
   GenerateGlobalTypes (source);
   GenerateGlobalDecls (source);

   g_generatorList.GenerateGlobal (source);

   if (BE_Globals::gen_externalization)
   {
      os << nl;

      for (bit = streamOps.begin(); bit != streamOps.end(); bit++)
      {
         (*bit)->GenerateStreamOps(source);
      }
   }

   os << nl;

   for (git = globalDecls.begin(); git != globalDecls.end(); git++)
   {
      (*git)->GenerateGlobalDecls (source);
   }

   os << nl;

   for (bit = typedefs.begin(); bit != typedefs.end(); bit++)
   {
      (*bit)->GenerateGlobalTypedef (source);
   }

   os << nl;

   if(BE_Globals::isocpp_test_methods)
   {
      //Generate file to stop missing file error
      DDS_StdString BaseFilename;
      BaseFilename = StripExtension(source.Filename());
      BaseFilename += "_testmethod.h";
      be_Source testsource;
      if(!testsource.Open(BaseFilename))
        cerr << "Cannot open: " << BaseFilename << endl;
      testsource.Close();

      //os << nl << "#ifndef " << Ifndefize(BE_Globals::ClientHeaderFilename + "DCPS");
      os << nl << "#ifndef " << Ifndefize(StripExtension(FindFilename(source.Filename())) + "DCPS.h");
      os << nl << "#ifndef " << Ifndefize(StripExtension(FindFilename(source.Filename())) + "_testmethod.h");
      os << nl << "#define " << Ifndefize(StripExtension(FindFilename(source.Filename())) + "_testmethod.h");
      os << nl << "#include \"" << StripExtension(FindFilename(source.Filename())) + "_testmethod.h" << "\"";
      os << nl << "#endif " << nl << "#endif";

    }
   os << nl << "#endif " << nl;
   source.Close();
}

void be_root::Generate (be_ServerHeader& source)
{
   ostream & os = source.Stream();

   be_CodeGenerator::Generate (source);


   os << nl << "#endif" << nl;

   source.Close ();

#if defined(DDS_TIE_HEADER)

   ostream & tos = source.tieHeader.Stream ();
   tos << nl;
   tos << "#endif" << nl;
   tos << nl;
   source.tieHeader.Close ();
#endif

}

void be_root::Generate (be_ClientImplementation & source)
{
   ostream & os = source.Stream ();
   TList<be_CodeGenerator*>::iterator iit;
   TList<be_Type*>::iterator bit;

   for (iit = implementations.begin(); iit != implementations.end(); iit++)
   {
      (*iit)->Generate(source);
   }

   os << nl;

   be_CodeGenerator::Generate (source);

   os << nl;

   os << nl;
   source.Close();
}

void be_root::Generate (be_ServerImplementation& source)
{
   const DDS_StdString corbaException = BE_Globals::CorbaScope("Exception");
   ostream & os = source.Stream ();

   be_CodeGenerator::Generate (source);

   os << nl << endl;
   source.Close();
}

void be_root::GenerateDependants
(
   be_ClientHeader & source,
   const char* dep,
   const char * scope
)
{
   DDS_StdString depname (dep);
   DDS_StdString nuScope (scope);

   TList<be_sequence*>::iterator it = be_root::deferred_seqs.begin ();

   while (it != be_root::deferred_seqs.end ())
   {
      be_sequence& sequence = *(*it);

      if (sequence.BaseTypeName () == depname)
      {
         sequence.SetName (nuScope, sequence.LocalName());
         sequence.Generate (source);

         GenerateDependants (source, sequence.LocalName(), scope);
      }

      ++it;
   }
}

void be_root::FinishAllProtoTypeCodes ()
{
   TList<be_Type*>::iterator iter;

   for
   (
      iter = typesThatNeedProtoTypeCodesFinished.begin ();
      iter != typesThatNeedProtoTypeCodesFinished.end ();
      ++iter
   )
   {
      be_Type & beType = *(*iter);
      beType.FinishProtoTypeCode ();
   }
}
