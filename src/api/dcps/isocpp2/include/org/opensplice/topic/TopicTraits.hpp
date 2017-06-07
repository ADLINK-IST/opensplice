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


/**
 * @file
 */

#ifndef ORG_OPENSPLICE_TOPIC_TOPICTRAITS_HPP_
#define ORG_OPENSPLICE_TOPIC_TOPICTRAITS_HPP_

#include <string.h>
#include "u_dataReader.h"

namespace org
{
namespace opensplice
{
namespace topic
{

typedef v_copyin_result (*copyInFunction)(c_base base, const void *, void *);
typedef void            (*copyOutFunction)(const void *, void *);


template <class TOPIC> class TopicTraits
{
public:
    static ::org::opensplice::topic::DataRepresentationId_t getDataRepresentationId()
    {
        return ::org::opensplice::topic::INVALID_REPRESENTATION;
    }

    static ::std::vector<os_uchar> getMetaData()
    {
        return ::std::vector<os_uchar>();
    }

    static ::std::vector<os_uchar> getTypeHash()
    {
        return ::std::vector<os_uchar>();
    }

    static ::std::vector<os_uchar> getExtentions()
    {
        return ::std::vector<os_uchar>();
    }

    static const char *getKeyList()
    {
        return "ExampleKeylist";
    }

    static const char *getTypeName()
    {
        return "ExampleName";
    }

    static std::string getDescriptor()
    {
        return  "<ExampleType>";
    }

    static copyInFunction getCopyIn()
    {
        return NULL;
    }

    static copyOutFunction getCopyOut()
    {
        return NULL;
    }
};

}
}
}


#endif /* ORG_OPENSPLICE_TOPIC_TOPICTRAITS_HPP_ */
