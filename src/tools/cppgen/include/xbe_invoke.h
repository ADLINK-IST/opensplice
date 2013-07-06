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
#ifndef _XBE_INVOKE_HH
#define _XBE_INVOKE_HH

#include "StdString.h"
#include "idl.h"
#include "xps_vector.h"
#include "xbe_codegen.h"
#include "xbe_opnameset.h"

// generator for the _invoke() function.  _invoke() runs on the server,
// is passed a request, and calls the correct dispatcher for the request.

class be_Invoke : public be_ServerGenerator
{
public:

   be_Invoke (DDS_StdString cppImplClassName, be_OpNameSet& opSet);
   virtual ~be_Invoke ();

   // BE_CODEGENERATOR VIRTUALS

   virtual void Generate (be_ServerHeader& serverHeader);
   virtual void Generate (be_ServerImplementation& serverImpl);

private:
   //
   // OpMapElementPtr holds a be_OpMapElement in a way that lets us
   // sort a vector of them
   //

   struct OpMapElementPtr
   {
      const be_OpMapElement* ptr;

      OpMapElementPtr(const be_OpMapElement& elem) : ptr(&elem)
      { }

      inline operator const be_OpMapElement*() const
      {
         return ptr;
      }

      inline const be_OpMapElement* operator->() const
      {
         return ptr;
      }

      inline bool operator<(const OpMapElementPtr& that) const
      {
         return strcmp(ptr->m_opDispatchName, that.ptr->m_opDispatchName) < 0;
      }
   };

   //
   // Range and Decision help us figure out where the dividing points are
   // in a range of array of be_OpMapElements
   //

   struct Range
   {
      char c;
      unsigned int lb;
      unsigned int ub;

      Range(char c_, unsigned int lb_, unsigned int ub_) : c(c_), lb(lb_), ub(ub_)
      { }

   }

   ;

   class Decision;

   friend class be_Invoke::Decision;

   class Decision
            : public DDSVector<Range*>
   {

   public:
      Decision(
         unsigned int lb,
         unsigned int ub,
         const DDSVector<OpMapElementPtr>& array);
      ~Decision();
      inline unsigned int DecisionIndex()
      {
         return m_charIndex;
      }

   private:
      unsigned int FindCharIndexOfNextDecision(unsigned int ub, unsigned int lb);
      void AddChar(char c);

      const DDSVector<OpMapElementPtr>& m_array;
      Range* m_currentRange;
      unsigned int m_arrayIndex;
      unsigned int m_charIndex;
   };

   //
   // TreeNode and TreeNodeAssoc hold the decision tree from which
   // NodeToCode() generates code
   //

   struct TreeNode;

   friend struct be_Invoke::TreeNode;

   struct TreeNodeAssoc;

   friend struct be_Invoke::TreeNodeAssoc;

   struct TreeNodeAssoc // a character and its associated TreeNode
   {
      char c;
      TreeNode *node;

      TreeNodeAssoc(char c_, TreeNode* node_) : c(c_), node(node_)
      { }

   }

   ;

   class TreeNodeAssocVec // a vector of TreeNodeAssocs
            : public DDSVector<TreeNodeAssoc*>
   {

   public:
      ~TreeNodeAssocVec();
   };

   struct TreeNode
   {
      const be_OpMapElement* op;  // op to invoke, if this is a leaf node;
      // otherwise NULL

      // remaining members describe a decision
      unsigned int decisionIndex;    // index of char to check for decision
      TreeNodeAssocVec* assocvec; // all the options, one per char value

      TreeNode(const be_OpMapElement* op); // make a leaf node
      TreeNode(          // make a decision node
         unsigned int index,
         TreeNodeAssocVec* avec);     // must not be null
      ~TreeNode();
   };

   // be_Invoke private functions

   TreeNode* CreateNode(unsigned int lb, unsigned int ub) const;
   void NodeToCode(ostream& os, be_Tab& tab, const TreeNode* node) const;
   void MakeElemArray();
   inline unsigned int NumberOfOps() const
   {
      return m_elemArray.size();
   }

   static DDS_StdString CppChar(char c);

   void Declare(ostream& os, be_Tab& tab);
   void DeclareImpl(ostream& os, be_Tab& tab);
   void GenerateBody(ostream& os, be_Tab& tab);

   // private data members

   DDS_StdString m_cppImplClassName; // class that owns the invoker
   const be_OpNameSet& m_opSet; // set of all operations in implClass
   TreeNode* m_rootNode;   // decision tree that gets translated into
   // if stmts in generated code
   DDSVector<OpMapElementPtr> m_elemArray;

   // don't copy or assign
   be_Invoke& operator=(be_Invoke&);
   be_Invoke(be_Invoke&);

   // unit test

   friend class test_be_Invoke;
};

#endif // _XBE_INVOKE_HH
