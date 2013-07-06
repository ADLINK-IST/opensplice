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
#ifndef _XBE_SOURCE_HH
#define _XBE_SOURCE_HH

#include "cppgen_iostream.h"
#include "cppgen_fstream.h"
#include "be_extern.h"
#include "xbe_globals.h"

class be_Source
{
public:

   be_Source ();
   virtual ~be_Source ();

   pbbool Open (const DDS_StdString & mainFilename);
   void Close ();

   inline const DDS_StdString & Basename () const
   {
      return m_basename;
   }

   inline ostream& Stream ()
   {
      return os;
   }

   inline const DDS_StdString & Filename () const
   {
      return m_filename;
   }

   inline const DDS_StdString & CreationTime () const
   {
      return creationTime;
   }

   inline const DDS_StdString & Margin () const
   {
      return margin;
   }

   inline void Indent ()
   {
      margin += "   ";
   }

   void Outdent ();
   void SetAccess (const DDS_StdString & access);

   virtual pbbool IsNotOkay ()
   {
      return (os ? FALSE : TRUE);
   }

   static const DDS_StdString PublicAccess;
   static const DDS_StdString ProtectedAccess;
   static const DDS_StdString PrivateAccess;
   static String_map idlIncludes;
   static String_map otherIncludes;

protected:

   DDS_StdString m_basename;
   DDS_StdString m_filename;
   DDS_StdString creationTime;
   DDS_StdString margin;
   ofstream os;
   pbbool closed;

private:

   friend void BE_produce ();
   static void Initialize ();
};

class be_ClientHeader : public be_Source
{
public:

   be_ClientHeader ();
   virtual ~be_ClientHeader ();

   pbbool Open(const DDS_StdString & mainFilename);

private:

   void GenerateSecondaryIncludes (ostream&);

   DDS_StdString ndefname;
};

class be_ClientImplementation : public be_Source
{
public:

   be_ClientImplementation ();
   virtual ~be_ClientImplementation ();

   pbbool Open (const DDS_StdString & mainFilename);
};

#if defined(DDS_TIE_HEADER)

class be_ServerTieHeader : public be_Source
{
public:

   DDS_StdString ndefname;
   DDS_StdString filename;

   be_ServerTieHeader ();
   virtual ~be_ServerTieHeader ();

   pbbool Open(const DDS_StdString & mainFilename);
};

#endif

class be_ServerHeader : public be_Source
{
public:

   DDS_StdString ndefname;

#if defined(DDS_TIE_HEADER)
   be_ServerTieHeader tieHeader;
#endif

   be_ServerHeader ();
   virtual ~be_ServerHeader ();

   pbbool Open (const DDS_StdString & mainFilename);
};

class be_ServerImplementation : public be_Source
{
public:

   be_ServerImplementation ();
   virtual ~be_ServerImplementation ();

   pbbool Open (const DDS_StdString & mainFilename);
};

class be_root;

class be_Noop
{
public:

   be_Noop (int) {};
};

class be_Tab
{

private:

#if !(defined(__SUNPRO_CC) && (__SUNPRO_CC > 0x500))

//   friend class ostream;
#endif

   be_Source & source;

public:

   be_Tab(const be_Source& _source_)
         :
         source(*(be_Source*)&_source_)
   {}

   inline const DDS_StdString&
   Margin() const
   {
      return source.Margin();
   }

   be_Noop indent()
   {
      source.Indent();
      return 0;
   }

   be_Noop outdent()
   {
      source.Outdent();
      return 0;
   }
};


inline ostream&
operator<<(ostream& os, const be_Tab& tab)
{
   os << tab.Margin();
   return os;
}

inline ostream&
operator<<(ostream& os, const be_Noop&)
{
   return os;
}

#endif
