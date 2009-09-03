#include "xbe_argument2.h"

be_Argument::be_Argument(
   const be_CppName& cppName,
   const be_DispatchableType& type,
   const be_ArgumentDirection& direction)
      :
      m_cppName(cppName),
      m_type(&type),
      m_direction(direction)
{ }

be_Argument::be_Argument(
   const be_argument& argument)
      :
      m_cppName((DDSString)argument.LocalName()),
      m_type(&argument.ArgumentType()),
      m_direction(argument.Direction())
{ }

be_Argument::be_Argument(
   const be_Argument& that)
      :
      m_cppName(that.m_cppName),
      m_type(that.m_type),
      m_direction(that.m_direction)
{ }

be_Argument&
be_Argument::operator=(const be_Argument& that)
{
   if (this != &that)
   {
      m_cppName = that.m_cppName;
      m_type = that.m_type;
      m_direction = that.m_direction;
   }

   return *this;
}
