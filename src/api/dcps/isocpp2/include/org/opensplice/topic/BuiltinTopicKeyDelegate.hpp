/*
*                         Vortex OpenSplice
*
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

#ifndef ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_KEY_DELEGATE_HPP_
#define ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_KEY_DELEGATE_HPP_

#include <kernelModule.h>


namespace org
{
namespace opensplice
{
namespace topic
{

class BuiltinTopicKeyDelegate
{
public:
    typedef uint32_t VALUE_T;
public:
    BuiltinTopicKeyDelegate() { }
    BuiltinTopicKeyDelegate(int32_t v[])
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

    void value(const int32_t v[])
    {
        key_[0] = v[0];
        key_[1] = v[1];
        key_[2] = v[2];
    }

    void value(const v_builtinTopicKey& key)
    {
        key_[0] = key.systemId;
        key_[1] = key.localId;
        key_[2] = key.serial;
    }

    bool operator ==(const BuiltinTopicKeyDelegate& other) const
    {
        return other.key_[0] == key_[0]
                 && other.key_[1] == key_[1]
                 && other.key_[2] == key_[2];
    }

private:
    int32_t key_[3];
};

}
}
}
#endif /* ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_KEY_DELEGATE_HPP_ */
