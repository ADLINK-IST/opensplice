#ifndef _FACE_COMMON_H_
#define _FACE_COMMON_H_

#include <dds/core/ddscore.hpp>

#ifdef NO_ERROR
/* This windows socket NO_ERROR return value can
 * clash with the FACE return code enum. */
#undef NO_ERROR
#endif

namespace FACE
{
    typedef int64_t SYSTEM_TIME_TYPE;

    const ::FACE::SYSTEM_TIME_TYPE INF_TIME_VALUE = (-1LL);

    typedef std::string CONFIGURATION_RESOURCE;

    enum RETURN_CODE_TYPE {
        NO_ERROR,
        NO_ACTION,
        NOT_AVAILABLE,
        ADDR_IN_USE,
        INVALID_PARAM,
        INVALID_CONFIG,
        PERMISSION_DENIED,
        INVALID_MODE,
        TIMED_OUT,
        MESSAGE_STALE,
        CONNECTION_IN_PROGRESS,
        CONNECTION_CLOSED,
        DATA_BUFFER_TOO_SMALL
    };

    typedef std::string SYSTEM_ADDRESS_TYPE;

    typedef ::FACE::SYSTEM_TIME_TYPE TIMEOUT_TYPE;

    typedef int32_t MESSAGE_RANGE_TYPE;

}

#endif /* _FACE_COMMON_H_ */
