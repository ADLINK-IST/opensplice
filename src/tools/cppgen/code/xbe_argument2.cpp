/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
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
