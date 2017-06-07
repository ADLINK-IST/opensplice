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


namespace org
{
namespace opensplice
{
namespace topic
{

template <typename Topic>
struct topic_data_writer { };

template <typename Topic>
struct topic_data_reader { };

template <typename Topic>
struct topic_data_seq { };

}
}
}

/**
 *  @internal Maps Topic type struct specializations through to
 * SACPP generated type support, typed sequence, and
 * typed reader and write classes.
 * @param TOPIC The CCPP topic type.
 */
#define REGISTER_TOPIC_TRAITS(TOPIC)                    \
    namespace dds { namespace topic {                    \
    template<> struct topic_type_support<TOPIC> {            \
        typedef TOPIC##TypeSupport type;                \
    };                                \
    template<> struct is_topic_type<TOPIC> { enum {value = 1 }; };    \
    template<> struct topic_type_name<TOPIC> {            \
        static std::string value() {                    \
            static topic_type_support<TOPIC>::type ts;            \
            return ts.get_type_name();                    \
        }                                \
    };                                \
    } }                                    \
    \
    namespace org { namespace opensplice { namespace topic {        \
    template<> struct topic_data_writer<TOPIC> {                \
        typedef TOPIC##DataWriter type; \
    };                                    \
    \
    template<> struct topic_data_reader<TOPIC> {                \
        typedef TOPIC##DataReader type;                    \
    };                                    \
    template<> struct topic_data_seq<TOPIC> {                \
        typedef TOPIC##Seq type;                        \
        typedef TOPIC##Seq_uniq_ utype;                    \
    }; \
    } } }


#endif /* ORG_OPENSPLICE_TOPIC_TOPICTRAITS_HPP_ */
