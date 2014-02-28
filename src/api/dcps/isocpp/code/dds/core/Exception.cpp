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
