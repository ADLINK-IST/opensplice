/*
 * in_transportReceiver.c
 *
 *  Created on: Feb 10, 2009
 *      Author: frehberg
 */

/* interfaces */
#include "in__transportReceiver.h"
#include "in_transportReceiver.h"
#include "in__object.h"

/* implementation */


/** \brief init */
void
in_transportReceiverInitParent(
        in_transportReceiver _this,
        in_objectKind kind,
        in_objectDeinitFunc deinit,
        in_transportReceiverPublicVTable staticPublicVTable)
{
    in_objectInit(OS_SUPER(_this), kind, deinit);
    _this->publicVTable = staticPublicVTable;
}

/** \brief deinit */
void
in_transportReceiverDeinit(
        in_transportReceiver _this)
{
    in_objectDeinit(OS_SUPER(_this));
}


/** \brief virtual function */
in_abstractReceiveBuffer
in_transportReceiverReceive(
        in_transportReceiver _this,
        os_boolean *isControl,
        os_time *timeout,
        in_result *errorState)
{
    in_abstractReceiveBuffer result = NULL;
    *errorState = IN_RESULT_OK;
    if (_this->publicVTable->receive) {
        result = _this->publicVTable->receive(_this, isControl, timeout, errorState);
    }
    return result;
}

/** \brief virtual function */
void
in_transportReceiverPeriodicAction(
		in_transportReceiver _this,
		os_time *timeout)
{
	if (_this->publicVTable->periodicAction) {
		_this->publicVTable->periodicAction(_this, timeout);
	}
}

/** \brief virtual function */
void
in_transportReceiverReleaseBuffer(
		in_transportReceiver _this,
		in_abstractReceiveBuffer buffer)
{
	if (_this->publicVTable->releaseBuffer) {
		_this->publicVTable->releaseBuffer(_this, buffer);
	}
}

/** \brief virtual function */
in_locator
in_transportReceiverGetDataUnicastLocator(
		in_transportReceiver _this)
{
	in_locator result = NULL;
	if (_this->publicVTable->getDataUnicastLocator) {
		result = _this->publicVTable->getDataUnicastLocator(_this);
	}
	return result;
}

/** \brief virtual function */
in_locator
in_transportReceiverGetDataMulticastLocator(
		in_transportReceiver _this)
{
	in_locator result = NULL;
	if (_this->publicVTable->getDataMulticastLocator) {
		result = _this->publicVTable->getDataMulticastLocator(_this);
	}
	return  result;
}

/** \brief virtual function */
in_locator
in_transportReceiverGetCtrlUnicastLocator(
		in_transportReceiver _this)
{

	in_locator result = NULL;
	if (_this->publicVTable->getCtrlUnicastLocator) {
		result = _this->publicVTable->getCtrlUnicastLocator(_this);
	}
	return  result;
}

/** \brief virtual function */
in_locator
in_transportReceiverGetCtrlMulticastLocator(
        in_transportReceiver _this)
{

    in_locator result = NULL;
    if (_this->publicVTable->getCtrlMulticastLocator) {
        result = _this->publicVTable->getCtrlMulticastLocator(_this);
    }
    return  result;
}

