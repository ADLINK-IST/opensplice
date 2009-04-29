/*
 * in_streamReader.c
 *
 *  Created on: Mar 5, 2009
 *      Author: frehberg
 */

/* **** interface headers **** */
#include "in__streamReader.h"
#include "in_report.h"
/* **** implementation headers **** */

/* **** private functions **** */

/* **** public functions **** */

/** */
os_boolean in_streamReaderInit(
        in_streamReader _this,
        in_objectKind kind,
        in_objectDeinitFunc deinit,
        in_streamReaderPublicVTable staticPublicVTable) {
    in_objectInit(OS_SUPER(_this), kind, deinit);
    _this->publicVTable = staticPublicVTable;

    return OS_TRUE;
}

/** */
void in_streamReaderDeinit(in_streamReader _this) {
    in_objectDeinit(OS_SUPER(_this));
}

/**
 * \param timeout if exceeded the operation will return with IN_RESULT_TIMEOUT
 *
 * The timeout shall be used to return from scan-operation
 * and check for termination. Or perform periodic actions.
 */
in_result
in_streamReaderScan(in_streamReader _this,
        in_streamReaderCallbackTable callbackTable,
        in_streamReaderCallbackArg callbackArg,
        os_time *timeout)
{
    in_result result = IN_RESULT_ERROR;

    if (_this && _this->publicVTable->scan) {
        /* typedef v_message  (*in_streamReaderScanFunc)(
         *							in_streamReader _this,
         *						    in_streamReaderHandler handler,
         *						    os_time timeout,
         *						    in_result *errorState);
         */
        result = _this->publicVTable->scan(
                _this,
                callbackTable,
                callbackArg,
                timeout);
    }

    return result;
}

