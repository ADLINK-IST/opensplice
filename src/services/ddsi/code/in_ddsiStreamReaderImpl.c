/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
/*
 * in_ddsiStreamReaderImpl.c
 *
 *  Created on: Mar 5, 2009
 *      Author: frehberg
 */

/* **** interface headers **** */
#include "in__ddsiStreamReaderImpl.h"

/* **** implementation headers **** */
#include "in_abstractReceiveBuffer.h"
#include "in_assert.h"
#include "in_report.h"
#include "in_ddsiSubmessageTokenizer.h"
#include "in_ddsiSubmessageDeserializer.h"
#include "in_ddsiElements.h"
#include "in_ddsiSubmessage.h"

/* **** private functions **** */

#define WARN_SUBMESSAGE_IGNORED(_kind)             \
	IN_REPORT_INFO_1(0, "submessage-kind %0x ignored", (os_uint32)(_kind))
#define WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(_kind) \
	IN_REPORT_INFO_1(0, "submessage-kind %0x not yet implemented", (os_uint32)(_kind))
#define WARN_SUBMESSAGE_UNKNOWN(_kind)            \
	IN_REPORT_INFO_1(0, "submessage-kind %0x not yet implemented", (os_uint32)(_kind))

#define IN_WARN_ENTITYKIND_UNKNOWN(_kind) \
	IN_REPORT_INFO_1(0, "entity-kind %0x unknown", (os_uint32)(_kind))

/** */
static os_boolean
in_ddsiStreamReaderImplProcessData(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageDeserializer submessageDeserializer);

/** */
static os_boolean
in_ddsiStreamReaderImplProcessHeartbeat(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageDeserializer submessageDeserializer);

/** */
static in_result
in_ddsiStreamReaderImplScanBuffer(
		in_ddsiStreamReaderImpl _this);

/** */
static os_boolean
in_ddsiStreamReaderImplProcessAppdefParticipant(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageData submessage);


/** */
static os_boolean
in_ddsiStreamReaderImplProcessAppdefReader(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageData submessage);


/** */
static os_boolean
in_ddsiStreamReaderImplProcessAppdefWriter(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageData submessage);


/** */
static os_boolean
in_ddsiStreamReaderImplProcessBuiltinParticipant(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageData submessage);


/** */
static os_boolean
in_ddsiStreamReaderImplProcessBuiltinReader(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageData submessage);


/** */
static os_boolean
in_ddsiStreamReaderImplProcessBuiltinWriter(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageData submessage);


/* *******************************************/
/* **** private function implementation **** */
/* *******************************************/

/** */
static os_boolean
in_ddsiStreamReaderImplProcessAppdefParticipant(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageData submessage)
{
	os_boolean result = OS_FALSE;

	return result;
}


/** */
static os_boolean
in_ddsiStreamReaderImplProcessAppdefReader(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageData submessage)
{
	os_boolean result = OS_FALSE;

	return result;
}

/** */
static os_boolean
in_ddsiStreamReaderImplProcessAppdefWriter(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageData submessage)
{
	os_boolean result = OS_FALSE;

	return result;
}


/** */
static os_boolean
in_ddsiStreamReaderImplProcessBuiltinParticipant(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageData submessage)
{
	os_boolean result = OS_FALSE;

	return result;
}


/** */
static os_boolean
in_ddsiStreamReaderImplProcessBuiltinReader(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageData submessage)
{
	os_boolean result = OS_FALSE;

	return result;
}


/** */
static os_boolean
in_ddsiStreamReaderImplProcessBuiltinWriter(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageData submessage)
{
	os_boolean result = OS_FALSE;

	return result;
}


/** */
static os_boolean
in_ddsiStreamReaderImplProcessData(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageDeserializer submessageDeserializer)
{
	os_boolean result = OS_TRUE;

	OS_STRUCT(in_ddsiSubmessageData) submessage;
	in_ddsiDeserializer deserializer = NULL;
	in_ddsiSubmessageHeader submessageHeader = NULL;

	assert(_this);

	deserializer =
		OS_SUPER(submessageDeserializer);
	submessageHeader =
		&(submessageDeserializer->submessageHeader);

	assert(deserializer);
	assert(submessageHeader);
	assert(submessageHeader->kind==IN_DATA);

	/* pre: parsing of submessage header succeed already,
	 * the deserializer is pointing to first octet behind the
	 * submessage-header. The deserializer prevents parsing
	 * beyond submessage end */

 	if (in_ddsiSubmessageDataInitFromBuffer(&submessage,
			submessageHeader,
			deserializer) < 0) {
		result = OS_FALSE;
	} else {
		in_ddsiEntityId writerId = &(submessage.writerId);

		switch (writerId->entityKind) {
		case IN_ENTITYKIND_APPDEF_UNKNOWN:           /* (0x00) */
			IN_WARN_ENTITYKIND_UNKNOWN(writerId->entityKind);

			break;

		case IN_ENTITYKIND_APPDEF_PARTICIPANT:        /* (0x01) */
			result =
				in_ddsiStreamReaderImplProcessAppdefParticipant(_this, &submessage);
			break;

		case IN_ENTITYKIND_APPDEF_WRITER_WITH_KEY:    /* (0x02) */
		case IN_ENTITYKIND_APPDEF_WRITER_NO_KEY:      /* (0x03) */
			result =
				in_ddsiStreamReaderImplProcessAppdefWriter(_this, &submessage);
			break;

		case IN_ENTITYKIND_APPDEF_READER_NO_KEY:      /* (0x04) */
		case IN_ENTITYKIND_APPDEF_READER_WITH_KEY:    /* (0x07) */
			result =
				in_ddsiStreamReaderImplProcessAppdefReader(_this, &submessage);
			break;

		case IN_ENTITYKIND_BUILTIN_PARTICIPANT:       /* (0xc1) */
			result =
				in_ddsiStreamReaderImplProcessBuiltinParticipant(_this, &submessage);
			break;
		case IN_ENTITYKIND_BUILTIN_WRITER_WITH_KEY:   /* (0xc2) */
		case IN_ENTITYKIND_BUILTIN_WRITER_NO_KEY:     /* (0xc3) */
			result =
				in_ddsiStreamReaderImplProcessBuiltinWriter(_this, &submessage);
			break;

		case IN_ENTITYKIND_BUILTIN_READER_NO_KEY:     /* (0xc4) */
		case IN_ENTITYKIND_BUILTIN_READER_WITH_KEY:   /* (0xc7) */
			result =
				in_ddsiStreamReaderImplProcessBuiltinReader(_this, &submessage);
			break;

		default:
			IN_WARN_ENTITYKIND_UNKNOWN(writerId->entityKind);
			result = OS_FALSE;
			break;
		}
	}
	return result;
}
/** */
static os_boolean
in_ddsiStreamReaderImplProcessHeartbeat(
        in_ddsiStreamReaderImpl _this,
        in_ddsiSubmessageDeserializer submessageDeserializer)
{
    os_boolean result = OS_FALSE;

    return result;
}

static in_result
in_ddsiStreamReaderImplScanBuffer(
		in_ddsiStreamReaderImpl _this)
{
	in_result result = IN_RESULT_OK;
	in_ddsiSubmessageTokenizer tokenizer =
		&(_this->currentReceiveBufferTokenizer);
	in_ddsiSubmessageToken token = NULL;
	OS_STRUCT(in_ddsiSubmessageDeserializer) deserializer;
	os_boolean continueScan = OS_TRUE;

	assert(_this->currentReceiveBuffer != NULL);

	/* iterate all submessages in buffer */
	for (token=in_ddsiSubmessageTokenizerGetNext(tokenizer, &deserializer);
		 token!=NULL && continueScan;
		 token=in_ddsiSubmessageTokenizerGetNext(tokenizer, &deserializer)) {

		const in_ddsiSubmessageKind kind =
			deserializer.submessageHeader.kind;

		switch (kind) {
		case  IN_SENTINEL:          /* (0x0) */
		    /* end of buffer reached */
			continueScan=OS_TRUE;
			break;

		case  IN_PAD:               /* (0x01) */
			WARN_SUBMESSAGE_IGNORED(kind);
			break;

		case  IN_DATA:              /* (0x02) */
		case  IN_NOKEY_DATA:        /* (0x03) */
			WARN_SUBMESSAGE_IGNORED(kind);
			break;

		case  IN_ACKNACK:           /* (0x06) */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;

		case  IN_HEARTBEAT:         /* (0x07) */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;

		case  IN_GAP:           	/* (0x08) */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;

		case  IN_INFO_TS:           /* (0x09) */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;

		case  IN_INFO_SRC:          /* (0x0c) */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;

		case  IN_INFO_REPLY_IP4:    /* (0x0d) */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;

		case  IN_INFO_DST:          /* (0x0e) */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;

		case  IN_INFO_REPLY:        /* (0x0f) */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;

		case  IN_DATA_FRAG:   		/* (0x10)  RTPS 2.0 Only */
			WARN_SUBMESSAGE_IGNORED(kind);
			break;

		case  IN_NOKEY_DATA_FRAG:   /* (0x11)  RTPS 2.0 Only */
			WARN_SUBMESSAGE_IGNORED(kind);
			break;

		case  IN_NACK_FRAG:   		/* (0x12)  RTPS 2.0 Only */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;

		case  IN_HEARTBEAT_FRAG:    /* (0x13)  RTPS 2.0 Only */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;

		case  IN_RTPS_DATA:   		/* (0x15)  RTPS 2.1 only */
			if (!in_ddsiStreamReaderImplProcessData(
					_this,
					&deserializer) ||
				result!=NULL) {
				/* interrupt scan either if parsing failed or result points to valid
				 * to new v_message object */
				continueScan = OS_FALSE;
			}
			break;

		case  IN_RTPS_DATA_FRAG:    /* (0x16)  RTPS 2.1 only */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;

		case  IN_ACKNACK_BATCH:     /* (0x17)  RTPS 2.1 only */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;

		case  IN_RTPS_DATA_BATCH:   /* (0x18)  RTPS 2.1 Only */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;

		case  IN_HEARTBEAT_BATCH:   /* (0x19)  RTPS 2.1 only */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;
		default:
			WARN_SUBMESSAGE_UNKNOWN(kind);
		}
	}

	return result;
}


static void
in_ddsiStreamReaderImplReleaseBuffer(
		in_ddsiStreamReaderImpl _this)
{
	if (_this->currentReceiveBuffer) {
		in_ddsiSubmessageTokenizerDeinit(
				&(_this->currentReceiveBufferTokenizer));
		in_ddsiReceiverDeinit(&(_this->receiver));

		in_transportReceiverReleaseBuffer(
				_this->transport,
				_this->currentReceiveBuffer);
		_this->currentReceiveBuffer = NULL;
	}
}
static os_boolean
in_ddsiStreamReaderImplFetchBuffer(
		in_ddsiStreamReaderImpl _this,
		os_time *timeout,
		in_result *errorState)
{
	os_boolean result = OS_TRUE;
	os_boolean isControl = OS_FALSE;
	*errorState = IN_RESULT_OK;

	if (_this->currentReceiveBuffer==NULL) {
		IN_REPORT_INFO(0, "waiting for receive-buffer");

		_this->currentReceiveBuffer =
			in_transportReceiverReceive(_this->transport,
					&isControl,
					timeout,
					errorState);
		/* currentReceiveBuffe!=NULL || errorState!=Ok */
		/* the transportReceiverReceive operation is only returned from
		 * if a message has been received, or an error occured. In the latter
		 * case the scan-operation shall be exited. */
		if (!_this->currentReceiveBuffer ||
			!in_ddsiReceiverInit(&(_this->receiver),
			        _this->currentReceiveBuffer) ||
			!in_ddsiSubmessageTokenizerInit(
					&(_this->currentReceiveBufferTokenizer),
					_this->currentReceiveBuffer)) {
			in_transportReceiverReleaseBuffer(
							_this->transport,
							_this->currentReceiveBuffer);
			_this->currentReceiveBuffer = NULL;
			result = OS_FALSE;
		}
	}
	return result;
}

static void
in_ddsiStreamReaderImplSetCallbackTable(
        in_ddsiStreamReaderImpl _this,
        in_streamReaderCallbackTable callbackTable,
        in_streamReaderCallbackArg   callbackArg)
{
    assert(_this->callbackTable == NULL);
    assert(_this->callbackArg == NULL);

    _this->callbackTable == callbackTable;
    _this->callbackArg == callbackArg;
}

static void
in_ddsiStreamReaderImplResetCallbackTable(
        in_ddsiStreamReaderImpl _this)
{
    _this->callbackTable == NULL;
    _this->callbackArg == NULL;
}
static in_result
in_ddsiStreamReaderImplScan(
		in_ddsiStreamReaderImpl _this,
        in_streamReaderCallbackTable callbackTable,
        in_streamReaderCallbackArg   callbackArg,
		os_time *timeout)
{
	in_result result = IN_RESULT_OK;
	os_time now = os_timeGet();
	os_time deadline = os_timeAdd(now, *timeout);
	os_time recvTimeout;
	os_boolean continueLoop = OS_TRUE;
	/* init */

	in_ddsiStreamReaderImplSetCallbackTable(_this,
	        callbackTable,
	        callbackArg);

	/* iterate: wait for messages until timeout exceeded */
	do {
		/* each iteration the receiveTimeout decreases, "now" guarantees
		 * positive receiveTimeout */
		recvTimeout = os_timeSub(deadline, now);
		assert(recvTimeout.tv_nsec > 0 && recvTimeout.tv_sec > 0);

		IN_REPORT_INFO(0, "Fetching buffer");
		if (!in_ddsiStreamReaderImplFetchBuffer(_this, &recvTimeout, &result)) {
			IN_REPORT_INFO(0, "Fetching buffer returned without buffer");
			/* either timeout or criticial error, such as network down */
			continueLoop = OS_FALSE;
		} else {
			IN_REPORT_INFO(0, "Scanning buffer");
			result = in_ddsiStreamReaderImplScanBuffer(_this);

			IN_REPORT_INFO(0, "Releasing buffer");
			/* all messages have been parsed from the buffer */
			in_ddsiStreamReaderImplReleaseBuffer(_this);

			/* update 'now' for next iteration */
			now = os_timeGet();
		}
	} while (continueLoop &&
			 os_timeCompare(now, deadline) == OS_LESS);


    in_ddsiStreamReaderImplResetCallbackTable(_this);

    return result;
}



static in_result
in_streamReaderScan_i(
		in_streamReader _this,
		in_streamReaderCallbackTable callbackTable,
		in_streamReaderCallbackArg   callbackArg,
		os_time *timeout)
{
	/* narrow _this and invoke scan operation */
	return in_ddsiStreamReaderImplScan(
			in_ddsiStreamReaderImpl(_this),
			callbackTable,
			callbackArg,
			timeout);
}

/* **** public functions **** */

/* static vtable */
static  OS_STRUCT(in_streamReaderPublicVTable) staticPublicVTable = {
        in_streamReaderScan_i
};

/** called by deriving classes */
os_boolean
in_ddsiStreamReaderImplInit(
        in_ddsiStreamReaderImpl _this,
        in_objectKind kind,
        in_objectDeinitFunc deinit,
        in_configChannel config,
        in_transportReceiver receiver,
        in_connectivityAdmin connectivityAdmin)
{

    in_streamReaderInit(OS_SUPER(_this),
            kind,
            deinit,
            &staticPublicVTable);

    _this->connectivityAdmin = connectivityAdmin;
    _this->transport = receiver;
    _this->currentReceiveBuffer = NULL;

    return OS_TRUE;
}

/** */
void
in_ddsiStreamReaderImplDeinit(
        in_ddsiStreamReaderImpl _this)
{
    _this->connectivityAdmin = NULL;
    _this->transport = NULL;
    in_objectDeinit(OS_SUPER(_this));
}



in_ddsiStreamReaderImpl
in_ddsiStreamReaderImplNew(
        in_configChannel config,
        in_transportReceiver receiver,
        in_connectivityAdmin connectivityAdmin)
{
    in_ddsiStreamReaderImpl result =
        os_malloc(sizeof(*result));



    if (result) {
        if (!in_ddsiStreamReaderImplInit(
                result,
                IN_OBJECT_KIND_STREAM_READER_BASIC,
                (in_objectDeinitFunc) in_ddsiStreamReaderImplDeinit,
                config,
                receiver,
                connectivityAdmin)) {
            os_free(result);
            result = NULL;
        }
    }
    return result;
}



#undef WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED
#undef WARN_SUBMESSAGE_UNKNOWN
#undef WARN_SUBMESSAGE_IGNORED
#undef WARN_ENTITY_KIND_UNKNOWN
