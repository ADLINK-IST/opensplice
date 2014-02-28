/*
*                         OpenSplice DDS
*
*   This software and documentation are Copyright 2006 to 2012 PrismTech
*   Limited and its licensees. All rights reserved. See file:
*
*                     $OSPL_HOME/LICENSE
*
*   for full copyright notice and license terms.
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
