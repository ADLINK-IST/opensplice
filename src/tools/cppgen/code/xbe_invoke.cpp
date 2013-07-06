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
#include "xbe_invoke.h"
#include "xbe_source.h"
#include "xbe_utils.h"

be_Invoke::be_Invoke
(
   DDS_StdString cppImplClassName,
   be_OpNameSet& opSet
)
:
   m_cppImplClassName (cppImplClassName),
   m_opSet (opSet),
   m_rootNode (NULL)
{
   MakeElemArray ();
}

be_Invoke::~be_Invoke ()
{
   delete m_rootNode;
}

void be_Invoke::Generate (be_ServerImplementation & serverImpl)
{
   ostream& os = serverImpl.Stream ();
   be_Tab tab = serverImpl;

   DeclareImpl (os, tab);

   os << tab << "{" << nl;
   tab.indent ();
   GenerateBody (os, tab);
   tab.outdent ();
   os << tab << "}" << nl << nl;
}

void be_Invoke::Generate (be_ServerHeader & serverHeader)
{
   ostream& os = serverHeader.Stream ();
   be_Tab tab = serverHeader;
   Declare (os, tab);
}

void be_Invoke::DeclareImpl (ostream& os, be_Tab& tab)
{
   os << "DDS::Codec::DispatchFN " << m_cppImplClassName 
      << "::_invoke (DDS::Codec::Request & _req" 
      << XBE_Ev::arg (XBE_ENV_ARGN) << ")" << nl;
}

void be_Invoke::Declare (ostream& os, be_Tab& tab)
{
   os << tab
      << "virtual DDS::Codec::DispatchFN _invoke ("
      << "DDS::Codec::Request & _req"
      << XBE_Ev::arg (XBE_ENV_ARGN) << ")" << nl;
}

/* Please consult me before modifying this code. Steve */

void be_Invoke::GenerateBody (ostream& os, be_Tab& tab)
{
   int ops = NumberOfOps ();

   if (ops)
   {
      os << tab << "void * ptr = this;" << nl;
      os << tab << "DDS::Codec::DispatchFN disp = (DDS::Codec::DispatchFN) NULL;" << nl;

      os << nl;

      // Generate switch statement

      if (!m_rootNode)
      {
         m_rootNode = CreateNode (0, m_elemArray.size () - 1);
      }
      NodeToCode (os, tab, m_rootNode);
   
      // Call the dispatcher

      os << tab;
      if (ops > 1)
      {
         os << "if (disp) (*disp) (ptr, _req";
      }
      else
      {
         os << "(*disp) (ptr, _req";
      }
      os << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
      os << tab << "return disp;" << nl;
   }
   else
   {
      os << tab << "return NULL;" << nl;
   }
}

// make elemArray into a sorted version of m_opSet

void be_Invoke::MakeElemArray ()
{
   assert(m_elemArray.size() == 0);

   be_OpNameSet::Be_OpMap::iterator iter;

   for (iter = m_opSet.begin(); iter != m_opSet.end(); ++iter)
   {
      OpMapElementPtr elem(*iter);
      m_elemArray.push_back(elem);
   }

   m_elemArray.sort();
}

// create a TreeNode and all subnodes for elements of m_elemArray between
// indices lb and ub inclusive

be_Invoke::TreeNode*
be_Invoke::CreateNode(unsigned int lb, unsigned int ub) const // recursive
{
   // if only one element in range, make a leaf node

   if (lb == ub)
   {
      return new TreeNode(m_elemArray[lb]);
   }

   // otherwise make a decision node

   Decision decision(lb, ub, m_elemArray);

   TreeNodeAssocVec* assocvec = new TreeNodeAssocVec;

   TreeNode* result = new TreeNode(decision.DecisionIndex(), assocvec);

   Decision::iterator iter;

   for (iter = decision.begin(); iter != decision.end(); ++iter)
   {
      TreeNodeAssoc* assoc = new TreeNodeAssoc(
                                (*iter)->c, CreateNode((*iter)->lb, (*iter)->ub));
      assocvec->push_back(assoc);
   }

   return result;
}

void be_Invoke::NodeToCode
(
   ostream & os,
   be_Tab & tab,
   const TreeNode * node
) const
{
   if (node->op)  // leaf node
   {
      os << tab << "disp = " << node->op->m_opdispatcher << ";" << nl;
      if (node->op->m_implname != m_cppImplClassName)
      {
         os << tab << "ptr = (" << node->op->m_implname << "*) this;" << nl;
      }
   }
   else    // decision node: generate switch() statement
   {
      assert (node->assocvec->size() >= 2);

      os << tab << "switch (_req.get_opname()[" << node->decisionIndex << "])" << nl;
      os << tab << "{" << nl;
      tab.indent ();

      for (DDS::ULong i = 0; i < node->assocvec->size(); i++)
      {
         const TreeNodeAssoc& assoc = *(*node->assocvec)[i];

         os << tab << "case '" << CppChar (assoc.c) << "':" << nl;

         // Generate the statement within the case

         tab.indent ();
         NodeToCode (os, tab, assoc.node);
         os << tab << "break;" << nl;
         tab.outdent ();
      }
      tab.outdent ();
      os << tab << "}" << nl;
   }
}

DDS_StdString be_Invoke::CppChar (char c)
{
   static char buf[2] = { 0, 0 };
   DDS_StdString result("");

   if (c == 0)
   {
      result = "\\0";
   }
   else if (c == '\\')
   {
      result = "\\";
   }
   else
   {
      assert(isprint(c));
      buf[0] = c;
      result = (const char*)buf;
   }

   return result;
}

// be_Invoke::TreeNode implementation ---------------------------------

be_Invoke::TreeNode::TreeNode(
   const be_OpMapElement* op_)
      :
      op(op_),
      decisionIndex(0),
      assocvec(NULL)
{}

be_Invoke::TreeNode::TreeNode(
   unsigned int index,
   TreeNodeAssocVec* vec)
      :
      op(NULL),
      decisionIndex(index),
      assocvec(vec)
{
   assert(vec != NULL);
}

be_Invoke::TreeNode::~TreeNode()
{
   delete assocvec;
}

be_Invoke::TreeNodeAssocVec::~TreeNodeAssocVec()
{
   iterator iter;

   for (iter = begin(); iter != end(); ++iter)
   {
      delete *iter;
   }
}

// Decision implementation --------------------------------------------

// constructor creates vector of Ranges

be_Invoke::Decision::Decision(
   unsigned int lb,    // lower bound of range to break up
   unsigned int ub,    // upper bound of range to break up

   const DDSVector<OpMapElementPtr>& array) // array that lb and ub index into
      :
      m_array(array),
      m_currentRange(NULL),
      m_arrayIndex(lb),
      m_charIndex(FindCharIndexOfNextDecision(lb, ub))
{
   assert(lb != ub); // Decision is only for splitting up a range

   for (unsigned int i = lb; i <= ub; i++)
   {
      AddChar(m_array[i]->m_opDispatchName[m_charIndex]);
   }
}

unsigned int
be_Invoke::Decision::FindCharIndexOfNextDecision(unsigned int lb, unsigned int ub)
{
   assert(lb != ub); // Decision is only for splitting up a range

   unsigned int charIndex = 0;

   for ( ; ; charIndex++)
   {
      char firstChar = m_array[lb]->m_opDispatchName[charIndex];

      for (unsigned int arrayIndex = lb + 1; arrayIndex <= ub; ++arrayIndex)
      {
         if (m_array[arrayIndex]->m_opDispatchName[charIndex] != firstChar)
         {
            return charIndex;
         }
      }
   }

   assert(false);
}

void be_Invoke::Decision::AddChar (char c)
{
   if (!m_currentRange || m_currentRange->c != c)
   {
      m_currentRange = new Range (c, m_arrayIndex, m_arrayIndex);
      push_back (m_currentRange);
   }
   else
   {
      ++m_currentRange->ub;
   }

   ++m_arrayIndex;
}

be_Invoke::Decision::~Decision ()
{
   iterator iter = begin ();

   while (iter != end ())
   {
      delete *iter;
      ++iter;
   }
}
