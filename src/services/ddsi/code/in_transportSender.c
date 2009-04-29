/*
 * in_transportSender.c
 *
 *  Created on: Feb 10, 2009
 *      Author: frehberg
 */

/* interfaces */
#include "in__transportSender.h"

/* implementation */
#include "in__object.h"


/** \brief init */
void
in_transportSenderInitParent(
		in_transportSender _this,
		in_objectKind kind,
		in_objectDeinitFunc deinit,
		/* must reference a static memory area*/
		in_transportSenderPublicVTable publicVTable)
{
	in_objectInit(OS_SUPER(_this), kind, deinit);
	_this->publicVTable = publicVTable;
}

/** \brief deinit */
void
in_transportSenderDeinit(
		in_transportSender _this)
{
	in_objectDeinit(OS_SUPER(_this));
}


/** \brief virtual function */
in_abstractSendBuffer
in_transportSenderGetBuffer(
		in_transportSender _this)
{
	in_abstractSendBuffer result = NULL;

	if (_this->publicVTable->getBuffer) {
		result = _this->publicVTable->getBuffer(_this);
	}

	return result;
}

/** \brief virtual function */
os_result
in_transportSenderSendTo(
		in_transportSender _this,
		in_locator sendTo,
		in_abstractSendBuffer buffer,
		os_size_t length,
		os_boolean isControl,
		os_time *timeout)
{
	os_result result = os_resultFail;

	if (_this->publicVTable->sendTo) {
		result = _this->publicVTable->sendTo(_this, sendTo, buffer, length, isControl, timeout);
	}
	return result;
}

/** \brief virtual function */
void
in_transportSenderPeriodicAction(
		in_transportSender _this,
		os_time *timeout)
{
	if (_this->publicVTable->periodicAction) {
		_this->publicVTable->periodicAction(_this, timeout);
	}
}

/** \brief virtual function */
void
in_transportSenderReleaseBuffer(
		in_transportSender _this,
		in_abstractSendBuffer buffer)
{
	if (_this->publicVTable->releaseBuffer) {
		_this->publicVTable->releaseBuffer(_this, buffer);
	}
}

/** \brief virtual function */
in_locator
in_transportSenderGetDataUnicastLocator(
        in_transportSender _this)
{
    in_locator result = NULL;
    if (_this->publicVTable->getDataUnicastLocator) {
        result = _this->publicVTable->getDataUnicastLocator(_this);
    }
    return result;
}

/** \brief virtual function */
in_locator
in_transportSenderGetDataMulticastLocator(
        in_transportSender _this)
{
    in_locator result = NULL;
    if (_this->publicVTable->getDataMulticastLocator) {
        result = _this->publicVTable->getDataMulticastLocator(_this);
    }
    return result;
}

/** \brief virtual function */
in_locator
in_transportSenderGetCtrlUnicastLocator(
        in_transportSender _this)
{
    in_locator result = NULL;
    if (_this->publicVTable->getCtrlUnicastLocator) {
        result = _this->publicVTable->getCtrlUnicastLocator(_this);
    }
    return result;
}
