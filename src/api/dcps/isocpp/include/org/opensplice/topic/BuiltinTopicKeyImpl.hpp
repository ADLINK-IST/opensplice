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

#ifndef ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_KEY_IMPL_HPP_
#define ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_KEY_IMPL_HPP_

namespace org
{
namespace opensplice
{
namespace topic
{

class BuiltinTopicKeyImpl
{
public:
    typedef uint32_t VALUE_T;
public:
    BuiltinTopicKeyImpl() { }
    BuiltinTopicKeyImpl(int32_t v[])
    {
        key_[0] = v[0];
        key_[1] = v[1];
        key_[2] = v[2];
    }
public:
    const int32_t* value() const
    {
        return key_;
    }

    void value(int32_t v[])
    {
        key_[0] = v[0];
        key_[1] = v[1];
        key_[2] = v[2];
    }

    bool operator ==(const BuiltinTopicKeyImpl& other) const
    {
        return other.key_ == key_;
    }

private:
    int32_t key_[3];
};

}
}
}
#endif /* ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_KEY_IMPL_HPP_ */
