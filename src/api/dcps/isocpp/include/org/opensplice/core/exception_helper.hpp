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

#ifndef ORG_OPENSPLICE_CORE_EXCEPTION_HELPER_HPP_
#define ORG_OPENSPLICE_CORE_EXCEPTION_HELPER_HPP_

#include <string>
#include <sstream>
#include <org/opensplice/core/config.hpp>
#include <dds/core/Exception.hpp>
#include <ios>
namespace org
{
namespace opensplice
{
namespace core
{

#define OSPL_INT_TO_STRING(n) OSPL_I_TO_STR(n)
#define OSPL_I_TO_STR(n) #n

/**
* @def OSPL_CONTEXT_LITERAL
* Platform independentish 'where are we now' macro. Expands to
* two comma separated stringish literals or const C strings. On some
* platforms the 'function' context is available as a preprocesser
* literalish token, on others it's a magic variable. On others: it's
* not available.
* @param foo Typically the action you are doing/have just done, or
* otherwise want to describe the context of. Should produce
* @code
 "foo at bar.cpp:123 in ", "int baz(int, char**)"
  @endcode
* with g++ for e.g. and something either more or less descriptive
* elsewhere. Hopefully.
* @see org::opensplice::core::context_to_string
* @see org::opensplice::core::check_and_throw
* @see org::opensplice::core::exception_helper
* @see org::opensplice::core::validate<T>
*/
/* Allegedly __FUNCTION__ has been in VS since 2005 */
#if defined (_MSC_VER)
#   define OSPL_CONTEXT_LITERAL(foo) foo " at " __FILE__ ":" OSPL_INT_TO_STRING(__LINE__) " in ",  __FUNCTION__ "()"
#elif defined(__GNUC__)
#   define OSPL_CONTEXT_LITERAL(foo) foo " at " __FILE__ ":" OSPL_INT_TO_STRING(__LINE__) " in ",  __PRETTY_FUNCTION__
#else
#   define OSPL_CONTEXT_LITERAL(foo) foo " at " __FILE__ ":" OSPL_INT_TO_STRING(__LINE__), ""
#endif

/**
* Produce a string suitable for populating an exception 'what' field.
* E.g:
* @code
throw dds::core::NullReferenceError(
                    org::opensplice::core::exception_helper(
                        OSPL_CONTEXT_LITERAL(
                            "dds::core::NullReferenceError : Unable to create ContentFilteredTopic. "
                            "Nil return from ::create_contentfilteredtopic"));
@endcode
* @param message A description of what went wrong / where we are
* @param function The function we are in. This is concatenated to message.
* @param ospl_error_info Should we try and include TSS OS_REPORT info about the
* last previous reported error condition.
* @param stack_trace Should we try and include a stack trace
* @return A string fit for passing to an exception constructor
*/
OSPL_ISOCPP_IMPL_API std::string exception_helper(const char* message,
        const char* function,
        bool ospl_error_info = true,
        bool stack_trace = true);

/**
* Produce a string suitable for populating an exception 'what' field.
* E.g:
* @code
std::string message = "dds::core::InvalidDataError";
message += context_to_string(OSPL_CONTEXT_LITERAL(""));
message += " Something bad happened. Some more information."
throw dds::core::NullReferenceError(
                    org::opensplice::core::exception_helper(message, false));
@endcode
* The above example shows how to produce an arbitrary-ish string. Note the
* non-default false second ospl_error_info argument that switches off the
* inclusion of the preceding erro info. This is appropriate if the error
* has not resulted from an underlying 'DCPS' call.
* @param message A description of what went wrong / where we are
* @param ospl_error_info Should we try and include TSS OS_REPORT info about the
* last previous reported error condition.
* @param stack_trace Should we try and include a stack trace
* @return A string fit for passing to an exception constructor
*/
OSPL_ISOCPP_IMPL_API std::string exception_helper(const std::string& message,
        bool ospl_error_info = true,
        bool stack_trace = true);

/**
* Turn an old school return code into it's string form, e.g.: DDS::RETCODE_OK,
* DDS::RETCODE_ALREADY_DELETED etc..
* @param code The return code to be demystified.
* @return A string corresponding to the code
*/
OSPL_ISOCPP_IMPL_API std::string dds_return_code_to_string(DDS::ReturnCode_t code);

/**
* Function to check an old school return code and throw the new PSM execption if
* appropriate. E.g.:
* @code
  check_and_throw_impl(result,
         context_to_string(
             OSPL_CONTEXT_LITERAL("Tried to create a topic")));
  @endcode
* @param code The value returned from the DCPS API function.
* @param context Some sort of clue to the receiver about what was
* called or what you were trying to do.
*/
OSPL_ISOCPP_IMPL_API void check_and_throw_impl(DDS::ReturnCode_t code,
        const std::string& context = "");

/**
 *  @internal Turns an OSPL_CONTEXT_LITERAL exapnasion into a std::string.
 * An OSPL_CONTEXT_LITERAL expands to two comma separated
 * stringish things. This function concatenates them.
 * @param context First stringish.
 * @param function Second stringish.
 * @see OSPL_CONTEXT_LITERAL
 */
inline std::string context_to_string(const char* context = "",
                                     const char* function = "")
{
    return std::string(context) += function;
}

/**
* Inline function to check an old school return code and throw the new
* PSM execption if appropriate. E.g.:
* @code
  check_and_throw(result,
                  OSPL_CONTEXT_LITERAL("Trying to create a topic"));
  @endcode
* @param code The value returned from the DCPS API function.
* @param context Some sort of clue to the receiver about what was
* called or what you were trying to do. Must be a literal or c_str.
* Defaults to "".
* @param function The function we were in. Must be a literal or c_str.
* Defaults to "".
* @see OSPL_CONTEXT_LITERAL
*/
inline void check_and_throw(DDS::ReturnCode_t code,
                            const char* context = "",
                            const char* function = "")
{
    if(code)
    {
        org::opensplice::core::check_and_throw_impl(code, context_to_string(context, function));
    }
}

/**
* Inline to check an old school return code and throw the new PSM execption if
* appropriate, taking a std::string message. E.g.:
* @code
  std::string my_message = "Oh noes, it's all gone wrong";
  check_and_throw(result, my_message);
  @endcode
* @param code The value returned from the DCPS API function.
* @param context Some sort of clue to the receiver about what was
* called or what you were trying to do.
*/
inline void check_and_throw(DDS::ReturnCode_t code,
                            const std::string& context)
{
    if(code)
    {
        org::opensplice::core::check_and_throw_impl(code, context);
    }
}

/**
* Check if the TIMEISH value supplied is at all suitable for arithmetic jiggery
* pokery. Invalidity encompasses (but is not restricted to) a -1 seconds part
* an 'infinity' Duration, an 'invalid' Time (or Duration).
* @param t the TIMEISH thing to check
* @return true if the argument is not suitable for doing sums with.
*/
template <typename TIMEISH>
bool is_valid_for_arithmetic(const TIMEISH& t)
{
    return (t.sec() != -1 // Invalid
            && t.sec() != 0xFFFFFFFF // Infinity
            && t.nanosec() < 1000000000);  // Invalid & infinity are > 10^9
}

/**
* Check a TIMEISH is valid for doing sums with.
* @param t the TIMEISH thing to check
* @param context Some sort of clue to the receiver about what was
* called or what you were trying to do. Must be a literal or c_str.
* Defaults to "".
* @param function String to be concateneated onto context.
* Must be a literal or c_str. Defaults to "".
* @throws a dds::core::InvalidDataError if not valid.
* @see OSPL_CONTEXT_LITERAL
* @see is_valid_for_arithmetic
*/
template <typename TIMEISH>
void validate(const TIMEISH& t, const char* context = "", const char* function = "")
{
    if(! is_valid_for_arithmetic<TIMEISH>(t))
    {
        std::stringstream message("dds::core::InvalidDataError");
        message << "Value invalid for arithmetic operations" << context << function
                << " seconds=" << t.sec() << " (" << std::hex << t.sec()
                << ") nanoseconds=" << t.nanosec() << " (" << std::hex << t.nanosec() << ")";
        throw dds::core::InvalidDataError(org::opensplice::core::exception_helper(message.str(), false));
    }
}

}
}
}

#endif /* ORG_OPENSPLICE_CORE_EXCEPTION_HELPER_HPP_ */
