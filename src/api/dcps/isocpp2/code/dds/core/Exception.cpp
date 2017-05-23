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

#include <ostream>
#include <dds/core/Exception.hpp>

#define DEFINE_LE_EXCEPTION(EXP) \
    EXP::EXP (const std::string& what) : Exception(), std::logic_error(what) { } \
    EXP::EXP (const EXP& src) : Exception(), std::logic_error(src.what()) {} \
    EXP::~EXP () throw () { } \
    const char* EXP::what() const throw () { return this->std::logic_error::what(); }

#define DEFINE_RE_EXCEPTION(EXP) \
    EXP::EXP (const std::string& what) : Exception(), std::runtime_error(what) { } \
    EXP::EXP (const EXP& src) : Exception(), std::runtime_error(src.what()) {} \
    EXP::~EXP () throw () { } \
    const char* EXP::what() const throw () { return this->std::runtime_error::what(); }

namespace dds
{
namespace core
{

// --- Exception: ------------------------------------------------------------

Exception::Exception() { }

Exception::~Exception() throw() { }

DEFINE_LE_EXCEPTION(Error)
DEFINE_LE_EXCEPTION(InvalidDataError)
DEFINE_LE_EXCEPTION(PreconditionNotMetError)
DEFINE_LE_EXCEPTION(UnsupportedError)
DEFINE_LE_EXCEPTION(NotEnabledError)
DEFINE_LE_EXCEPTION(InconsistentPolicyError)
DEFINE_LE_EXCEPTION(ImmutablePolicyError)
DEFINE_LE_EXCEPTION(AlreadyClosedError)
DEFINE_LE_EXCEPTION(IllegalOperationError)

DEFINE_RE_EXCEPTION(OutOfResourcesError)
DEFINE_RE_EXCEPTION(TimeoutError)
DEFINE_RE_EXCEPTION(InvalidDowncastError)
DEFINE_RE_EXCEPTION(NullReferenceError)

InvalidArgumentError::InvalidArgumentError(const std::string& what) : Exception(), std::invalid_argument(what) { }
InvalidArgumentError::InvalidArgumentError(const InvalidArgumentError& src) : Exception(), std::invalid_argument(src.what()) {}
InvalidArgumentError::~InvalidArgumentError() throw() {}
const char* InvalidArgumentError::what() const throw()
{
    return this->std::invalid_argument::what();
}

}
}
