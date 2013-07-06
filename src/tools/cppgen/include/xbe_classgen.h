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
#ifndef _XBE_CLASSGENERATOR_H_
#define _XBE_CLASSGENERATOR_H_

#include "be_extern.h"
#include "xbe_codegen.h"
#include "xbe_source.h"
#include "tlist.h"
#include "xbe_exceptionlist.h"

class be_ClassParent
{
public:

   enum CP_InheritType { IT_Private, IT_Protected, IT_Public };

   be_ClassParent
   (
      const DDS_StdString & nm,
      bool loc,
      bool virt,
      CP_InheritType itype = IT_Public
   )
   :
      isLocal (loc),
      isVirtual (virt),
      name (nm),
      type (itype)
   {}

   be_ClassParent
   (
      const DDS_StdString & nm,
      const DDS_StdString & scopedName,
      bool loc,
      bool virt,
      CP_InheritType itype = IT_Public
   )
   :
      isLocal (loc),
      isVirtual (virt),
      name (nm),
      scopedName (scopedName),
      type (itype)
   {}

   be_ClassParent (const be_ClassParent& that)
   :
      isLocal (that.isLocal),
      isVirtual (that.isVirtual),
      name (that.name),
      scopedName (that.scopedName),
      type (that.type)
   {}

   inline const DDS_StdString & Name () { return name; }

   inline const DDS_StdString & ScopedName ()
   {
      return scopedName;
   }

   inline bool local () { return isLocal; };

private:

   friend class be_ClassGenerator;

   bool isLocal;
   bool isVirtual;
   DDS_StdString name;
   DDS_StdString scopedName;
   CP_InheritType type;
};

class be_ClassMember
{

public:

   enum CM_Access { MA_Private, MA_Protected, MA_Public };

public:

   be_ClassMember(
      const DDS_StdString& _type_,
      const DDS_StdString& _name_,
      const DDS_StdString& _stOpType_,
      const DDS_StdString& _istOpType_,
      int _levelsOfAbstraction_ = 1,
      CM_Access _access_ = MA_Private)
         :
         type(_type_),
         name(_name_),
         accessorName(name),
         streamOpType(_stOpType_),
         istreamOpType(_istOpType_),
         levelsOfAbstraction(_levelsOfAbstraction_),
         access(_access_)
   {}

   be_ClassMember(
      const DDS_StdString& _type_,
      const DDS_StdString& _name_,
      const DDS_StdString& _stOpType_,
      const DDS_StdString& _istOpType_,
      const DDS_StdString& _accessorName_,
      CM_Access _access_ = MA_Private)
         :
         type(_type_),
         name(_name_),
         accessorName(_accessorName_),
         streamOpType(_stOpType_),
         istreamOpType(_istOpType_),
         access(_access_)
   {}

   be_ClassMember(const be_ClassMember& that)
         :
         type(that.type),
         name(that.name),
         streamOpType(that.streamOpType),
         istreamOpType(that.istreamOpType),
         access(that.access)
   {}

   inline const DDS_StdString& MemberType()
   {
      return type;
   }

   inline const DDS_StdString& StreamOpType()
   {
      return streamOpType;
   }

   inline const DDS_StdString& IStreamOpType()
   {
      return istreamOpType;
   }

   inline const DDS_StdString& Name()
   {
      return name;
   }

   inline const DDS_StdString& AccessorName()
   {
      return accessorName;
   }

   inline CM_Access Access()
   {
      return access;
   }

   inline int LevelsOfAbstraction() const
   {
      return levelsOfAbstraction;
   }

   // BE_CLASS_MEMBER VIRTUALS
   virtual be_ClassMember*
   Duplicate()
   {
      return new be_ClassMember(*this);
   }

   virtual ~be_ClassMember()
   {}

private:

   friend class be_ClassGenerator;

   DDS_StdString type;
   DDS_StdString name;
   DDS_StdString accessorName;
   DDS_StdString streamOpType;
   DDS_StdString istreamOpType;
   int levelsOfAbstraction;
   CM_Access access;
};


class be_Type;

class be_ClassGenerator
         :
         public be_CodeGenerator
{

public:

   enum ClassAccess { CA_PRIVATE, CA_PROTECTED, CA_PUBLIC, CA_UNDEFINED };

   void SetParents(const TList<be_ClassParent *> &_parents_);
   void SetMembers(const TList<be_ClassMember *> &_members_);

   inline virtual DDS_StdString
   LocalClassName() const
   {
      return className;
   }

   inline virtual DDS_StdString
   ScopedClassName() const
   {
      return scopedClassName;
   }

   DDS_StdString ClassScope() const;

   inline virtual int
   MemberCount() const
   {
      return members.size();
   }

   inline virtual int
   ParentCount() const
   {
      return parents.size();
   }

   inline void
   SetName(const DDS_StdString& _name_)
   {
      className = _name_;
   }

   inline void
   SetScopedClassName(const DDS_StdString& _name_)
   {
      scopedClassName = _name_;
   }

   inline void
   AddParent(be_ClassParent* parent)
   {
      if (parent)
      {
         parents.push_back(parent);
      }
   }

   inline void
   AddMember(be_ClassMember* member)
   {
      if (member)
      {
         members.push_back(member);
      }
   }

public:

   be_ClassGenerator();
   be_ClassGenerator(const DDS_StdString& _name_);
   be_ClassGenerator(const DDS_StdString& _name_,
                     const TList<be_ClassParent *> &_parents_,
                     const TList<be_ClassMember *> &_members_);
   virtual ~be_ClassGenerator();

   void SetAccess(be_Source& source, ClassAccess newAccess);

   // BE_CLASS_GENERATOR VIRTUALS
   virtual void GenerateOpenClassDefinition(be_ClientHeader& source);
   virtual void GenerateHierachySearch(be_ClientImplementation& source);
   virtual void GenerateClassDeclarations(be_ClientHeader& source);
   virtual void GenerateMemberDeclarations(be_ClientHeader& source);
   virtual void GenerateCloseClassDefinition(be_ClientHeader& source);
   virtual void GenerateConstructor(be_ClientHeader& source);
   virtual void GenerateAccessors(be_ClientHeader& source);
   virtual void GenerateDestructor(be_ClientHeader& source);

   virtual void GenerateDestructor(be_ClientImplementation& source);

   // BE_CODE_GENERATOR VIRTUALS
   virtual void Generate(be_ClientHeader& source);
   virtual void Generate(be_ClientImplementation&)
   {}

   virtual void Generate(be_ServerImplementation &)
   {}

   virtual void Generate(be_ServerHeader&)
   {}

private:

   static const char * ClassAccesses[CA_UNDEFINED];

   ClassAccess access;
   DDS_StdString className;
   DDS_StdString scopedClassName;
   TList<be_ClassParent *> parents;
   TList<be_ClassMember *> members;
};

class AST_Type;
class UTL_StrList;
class be_argument;

class be_OpStubGenerator : public be_CodeGenerator
{
public:

   typedef TList<be_argument*> ArgList;

   be_OpStubGenerator
   (
      const DDS_StdString & scopedClassname,
      const DDS_StdString & opKey,
      const DDS_StdString & opName,
      const DDS_StdString & signature,
      be_Type * returnType,
      pbbool isOneWay,
      const ArgList & arguments,
      UTL_ExceptList *exceptions,
      UTL_StrList * context,
      const DDS_StdString & opDispatchName
   );

   virtual ~be_OpStubGenerator();

   // BE_CLASS_GENERATOR VIRTUALS
   virtual void Generate(be_ClientHeader&)
   {}

   virtual void Generate(be_ClientImplementation&);
   virtual void Generate(be_ServerImplementation &)
   {}

   virtual void Generate(be_ServerHeader&)
   {}

private:

   DDS_StdString m_scopedClassname;
   DDS_StdString m_opKey;
   DDS_StdString m_opName;
   DDS_StdString m_signature;
   be_Type * m_returnType;
   pbbool m_isOneWay;
   ArgList m_arguments;
   be_ExceptionList m_exceptions;
   UTL_StrList * m_context;
   DDS_StdString m_opDispatchName;

   int InArgCount();
   int OutArgCount();
   pbbool HasReturn();
};

class be_AttStubGenerator
         :
         public be_CodeGenerator
{

public:

   be_AttStubGenerator
   (
      const DDS_StdString& scopedClassname,
      const DDS_StdString& opKey,
      const DDS_StdString& opName,
      const DDS_StdString& signature,
      be_Type * returnType,
      pbbool isSetAttribute
   );

   virtual ~be_AttStubGenerator();

   // BE_CLASS_GENERATOR VIRTUALS
   virtual void Generate(be_ClientHeader&)
   {}

   virtual void Generate(be_ClientImplementation&);
   virtual void Generate(be_ServerImplementation &)
   {}

   virtual void Generate(be_ServerHeader&)
   {}

private:

   DDS_StdString m_scopedClassname;
   DDS_StdString m_opKey;
   DDS_StdString m_opName;
   DDS_StdString m_signature;
   be_Type * m_returnType;
   pbbool m_isSetAttribute;
};

#endif //_XBE_CLASSGENERATOR_H_
