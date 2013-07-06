#ifndef OMG_DDS_CORE_EXCEPTION_HPP_
#define OMG_DDS_CORE_EXCEPTION_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdexcept>
#include <string>
#include <dds/core/macros.hpp>

#if defined _MSC_VER
#   pragma warning (push)
#   pragma warning (disable:4275) // non dll-interface class 'std::foo_error' used as base for dll-interface class 'dds::core::BarError'
#endif

namespace dds { namespace core {

  /**
   * @file
   * This file contains the exceptions corresponding to DDS errors.
   * In the DDS-PSM-Cxx in place of DDS errors the associated exception
   * should be raised. Please section 7.5.5 of the DDS-PSM-C++ specification.
   */

  class OMG_DDS_API Exception
  {
  protected:
    Exception();
  public:
    virtual ~Exception() throw ();

  public:
    virtual const char* what() const throw () = 0;
  };

  class OMG_DDS_API Error : public Exception, public std::logic_error
  {
  public:
    explicit Error(const std::string& msg);
    Error(const Error& src);
    virtual ~Error() throw ();

  public:
    virtual const char* what() const throw ();
  };


  class OMG_DDS_API AlreadyClosedError : public Exception, public std::logic_error
  {
  public:
    explicit AlreadyClosedError(const std::string& msg);
    AlreadyClosedError(const AlreadyClosedError& src);
    virtual ~AlreadyClosedError() throw ();

  public:
    virtual const char* what() const throw ();
  };


  class OMG_DDS_API IllegalOperationError : public Exception, public std::logic_error
  {
  public:
    explicit IllegalOperationError(const std::string& msg);
    IllegalOperationError(const IllegalOperationError& src);
    virtual ~IllegalOperationError() throw ();

  public:
    virtual const char* what() const throw ();
  };


  class OMG_DDS_API ImmutablePolicyError : public Exception, public std::logic_error
  {
  public:
    explicit ImmutablePolicyError(const std::string& msg);
    ImmutablePolicyError(const ImmutablePolicyError& src);
    virtual ~ImmutablePolicyError() throw ();

  public:
    virtual const char* what() const throw ();
  };


  class OMG_DDS_API InconsistentPolicyError : public Exception, public std::logic_error
  {
  public:
    explicit InconsistentPolicyError(const std::string& msg);
    InconsistentPolicyError(const InconsistentPolicyError& src);
    virtual ~InconsistentPolicyError() throw ();

  public:
    virtual const char* what() const throw ();
  };


  class OMG_DDS_API InvalidArgumentError : public Exception, public std::invalid_argument
  {
  public:
    explicit InvalidArgumentError(const std::string& msg);
    InvalidArgumentError(const InvalidArgumentError& src);
    virtual ~InvalidArgumentError() throw ();

  public:
    virtual const char* what() const throw ();
  };


  class OMG_DDS_API NotEnabledError : public Exception, public std::logic_error
  {
  public:
    explicit NotEnabledError(const std::string& msg);
    NotEnabledError(const NotEnabledError& src);
    virtual ~NotEnabledError() throw ();

  public:
    virtual const char* what() const throw ();
  };


  class OMG_DDS_API OutOfResourcesError : public Exception, public std::runtime_error
  {
  public:
    explicit OutOfResourcesError(const std::string& msg);
    OutOfResourcesError(const OutOfResourcesError& src);
    virtual ~OutOfResourcesError() throw ();

  public:
    virtual const char* what() const throw ();
  };


  class OMG_DDS_API PreconditionNotMetError : public Exception, public std::logic_error
  {
  public:
    explicit PreconditionNotMetError(const std::string& msg);
    PreconditionNotMetError(const PreconditionNotMetError& src);
    virtual ~PreconditionNotMetError() throw ();

  public:
    virtual const char* what() const throw ();
  };


  class OMG_DDS_API TimeoutError : public Exception, public std::runtime_error
  {
  public:
    explicit TimeoutError(const std::string& msg);
    TimeoutError(const TimeoutError& src);
    virtual ~TimeoutError() throw ();

  public:
    virtual const char* what() const throw ();
  };


  class OMG_DDS_API UnsupportedError : public Exception, public std::logic_error
  {
  public:
    explicit UnsupportedError(const std::string& msg);
    UnsupportedError(const UnsupportedError& src);
    virtual ~UnsupportedError() throw ();

  public:
    virtual const char* what() const throw ();
  };

  class OMG_DDS_API InvalidDowncastError : public Exception, public std::runtime_error
  {
  public:
    explicit InvalidDowncastError(const std::string& msg);
    InvalidDowncastError(const InvalidDowncastError& src);
    virtual ~InvalidDowncastError() throw();

  public:
    virtual const char* what() const throw ();
  };

  class OMG_DDS_API NullReferenceError : public Exception, public std::runtime_error
  {
  public:
    explicit NullReferenceError(const std::string& msg);
    NullReferenceError(const NullReferenceError& src);
    virtual ~NullReferenceError() throw();

  public:
    virtual const char* what() const throw ();
  };


  class OMG_DDS_API InvalidDataError : public Exception, public std::logic_error
  {
  public:
    explicit InvalidDataError(const std::string& msg);
    InvalidDataError(const InvalidDataError& src);
    virtual ~InvalidDataError() throw ();

  public:
    virtual const char* what() const throw ();
  };

}}

#if defined _MSC_VER
#   pragma warning (pop)
#endif


#endif /* OMG_DDS_CORE_EXCEPTION_HPP_ */
