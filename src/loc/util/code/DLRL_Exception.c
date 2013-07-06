/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "os_stdlib.h"
#include "DLRL_Exception.h"
#include "DLRL_Util.h"


void
DLRL_Exception_init(
    DLRL_Exception *_this)
{
  assert(_this);

    _this->exceptionID = DLRL_NO_EXCEPTION;
#ifndef NDEBUG
    memset(_this->exceptionMessage, 0, DLRL_MAX_EXCEPTION_MESSAGE_SIZE);
#endif
}

void
DLRL_Exception_format(
    DLRL_Exception *_this,
    LOC_long exceptionID,
    char *exceptionName,
    char *format,
    va_list args)
{
  char *p;
  int space = DLRL_MAX_EXCEPTION_MESSAGE_SIZE;
  int len;

  assert (_this);
  assert(exceptionName);
  assert(format);

  _this->exceptionID = exceptionID;
  p = _this->exceptionMessage;

  len = strlen(_this->exceptionMessage);
  space = DLRL_MAX_EXCEPTION_MESSAGE_SIZE - len;
  snprintf(p + len, space, "Exception %s --\n", exceptionName);
  p[DLRL_MAX_EXCEPTION_MESSAGE_SIZE - 1] = '\0';
  len = strlen(p);
  space = DLRL_MAX_EXCEPTION_MESSAGE_SIZE - len;

  if (format)
  {
    snprintf(p + len, space, "  ");
    p[DLRL_MAX_EXCEPTION_MESSAGE_SIZE - 1] = '\0';
    len = strlen(p);
    space = DLRL_MAX_EXCEPTION_MESSAGE_SIZE - len;
    os_vsnprintf(p + len, space, format, args);
    p[DLRL_MAX_EXCEPTION_MESSAGE_SIZE - 1] = '\0';
    len = strlen(p);
    space = DLRL_MAX_EXCEPTION_MESSAGE_SIZE - len;
    snprintf(p + len, space, "\n");
    p[DLRL_MAX_EXCEPTION_MESSAGE_SIZE - 1] = '\0';
  }
}

void
DLRL_Exception_throw(
    DLRL_Exception *_this,
    LOC_long exceptionID,
    char *exceptionName,
    ...)
{
  char *format;
  va_list args;

  assert(_this);
  assert(exceptionName);

  va_start(args, exceptionName);
  format = va_arg(args, char *);
  DLRL_Exception_format(_this, exceptionID, exceptionName, format, args);
  va_end(args);
}

void
DLRL_Exception_propagate(
    DLRL_Exception *_this,
    char *file,
    int line,
    char *function)
{
    int len;

    assert(_this);
    assert(file);
    assert(function);

    len = strlen(_this->exceptionMessage);
    snprintf(_this->exceptionMessage + len, DLRL_MAX_EXCEPTION_MESSAGE_SIZE - len,
           "  %s:%d:%s\n", file, line, function);
    _this->exceptionMessage[DLRL_MAX_EXCEPTION_MESSAGE_SIZE - 1] = '\0';
}

void
DLRL_Exception_transformResultToException(
    DLRL_Exception *_this,
    ...)
{
    char *format, new_format[200];
    va_list args;
    u_result result;

    assert(_this);

    va_start(args, _this);
    result = va_arg(args, u_result);
    if (result != U_RESULT_OK){
        format = va_arg(args, char *);

        snprintf(new_format, 200, "%s: %s", DLRL_Util_userResultToString(result), format);
        new_format[200 - 1] = '\0';
        DLRL_Exception_format(_this, DLRL_DCPS_ERROR, "DLRL_DCPS_ERROR", new_format, args);
    }
    va_end(args);
}

void
DLRL_Exception_transformWriteResultToException(
    DLRL_Exception *_this,
    ...)
{
    char *format, new_format[200];
    va_list args;
    v_writeResult result;

    assert(_this);

    va_start(args, _this);
    result = va_arg(args, v_writeResult);
    if(result != V_WRITE_SUCCESS){
        format = va_arg(args, char *);

        snprintf(new_format, 200, "%s: %s", DLRL_Util_writeResultToString(result), format);
        new_format[200 - 1] = '\0';
        DLRL_Exception_format(_this, DLRL_DCPS_ERROR, "DLRL_DCPS_ERROR", new_format, args);
    }
    va_end(args);
}

char *
DLRL_Exception_exceptionIDToString(
    LOC_long exceptionID)
{
    char *returnString = NULL;

    switch (exceptionID)
    {
        case DLRL_NO_EXCEPTION:
            returnString = "DLRL_NO_EXCEPTION";
            break;
        case DLRL_DCPS_ERROR:
            returnString = "DLRL_DCPS_ERROR";
            break;
        case DLRL_BAD_HOME_DEFINITION:
            returnString = "DLRL_BAD_HOME_DEFINITION";
            break;
        case DLRL_BAD_PARAMETER:
            returnString = "DLRL_BAD_PARAMETER";
            break;
        case DLRL_SQL_ERROR:
            returnString = "DLRL_SQL_ERROR";
            break;
        case DLRL_NOT_FOUND:
            returnString = "DLRL_NOT_FOUND";
            break;
        case DLRL_ALREADY_EXISTING:
            returnString = "DLRL_ALREADY_EXISTING";
            break;
        case DLRL_INVALID_OBJECTS:
            returnString = "DLRL_INVALID_OBJECTS";
            break;
        case DLRL_PRECONDITION_NOT_MET:
            returnString = "DLRL_PRECONDITION_NOT_MET";
            break;
        case DLRL_ALREADY_DELETED:
            returnString = "DLRL_ALREADY_DELETED";
            break;
        case DLRL_NO_SUCH_ELEMENT:
            returnString = "DLRL_NO_SUCH_ELEMENT";
            break;
        case DLRL_OUT_OF_MEMORY:
            returnString = "DLRL_OUT_OF_MEMORY";
            break;
        case DLRL_ERROR:
            returnString = "DLRL_ERROR";
            break;
        default:
            returnString = "Unknown Exception ID";
            break;
    }
    return returnString;
}
