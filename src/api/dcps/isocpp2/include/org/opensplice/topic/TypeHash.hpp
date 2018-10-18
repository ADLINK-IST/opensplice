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
#ifndef ORG_OPENSPLICE_TOPIC_TYPE_HASH_HPP_
#define ORG_OPENSPLICE_TOPIC_TYPE_HASH_HPP_

namespace org { namespace opensplice { namespace topic {

class TypeHash {
public:
    TypeHash() :
        msb_(0), lsb_(0) {}

    TypeHash(uint64_t msb, uint64_t lsb) :
        msb_(msb), lsb_(lsb) {}

    uint64_t msb() const {
        return msb_;
    }

    void msb(uint64_t msb) {
        msb_ = msb;
    }

    uint64_t lsb() const {
        return lsb_;
    }

    void lsb(uint64_t lsb) {
        lsb_ = lsb;
    }

    bool operator ==(const TypeHash& other) const
    {
        return other.msb_ == msb_ && other.lsb_ == lsb_;
    }

private:
    uint64_t msb_;
    uint64_t lsb_;
};

}}}

#endif /* ORG_OPENSPLICE_TOPIC_TYPE_HASH_HPP_ */
