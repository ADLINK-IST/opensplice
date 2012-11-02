/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef IN_LOCATOR_H_
#define IN_LOCATOR_H_

/* OS Abstraction includes */
#include "os_socket.h"

/* DDSi includes */
#include "in_commonTypes.h"
#include "in_ddsiSerializer.h"
#include "in_ddsiDeserializer.h"
#include "in_address.h"

/** \brief Class inherits from  in_object */

/** \brief User defined parameter walking a list */
typedef void* in_locatorActionArg;

/** \brief User defined walk operation */
typedef void (*in_locatorAction)(in_locator _this, in_locatorActionArg arg);

/* The usual cast-method for class in_locator. Note that because in_locator
 * does not contain any metadata there is no type checking performed.
 */
#define in_locator(locator) ((in_locator)locator)


/** \brief The constructor of in_locator object from address string  and port
 */
in_locator
in_locatorNewFromString(
    const os_char* addressString,
    os_uint32 port);

/** \brief Init as invalid (undefined) locator */
void
in_locatorInitInvalid(in_locator _this);

/** \brief Construct dynamic object
 *
 * \param port a value in range [1,65535]
 * \param address array of 4 octets for IPv4 and 16 octets for IPv6
 * */
in_locator
in_locatorNew(
    os_uint32 port,
    in_address address);

/** \brief This operation initializes the in_locator class.
 *
 * - It will set the reference count to 1.
 *
 * \param _this The object to initialize
 * \param sockAddr The internet location, in internet byteorder.
 */
void
in_locatorInitFromSockaddr(
    in_locator _this,
    const struct sockaddr *sockAddr);

/** \brief init operation for convenience */
void
in_locatorInitUDPv4(in_locator _this,
		os_uint32 port,
		const in_addressIPv4 address);

/** \brief init operation for convenience */
void
in_locatorInitUDPv6(in_locator _this,
		os_uint32 port,
		const in_addressIPv6 address);

/** \brief The destructor of in_locator object, decreasing refcount and free
 * if 0 is reached
 */
void
in_locatorFree(
    in_locator _this);

/** \brief Increment refcounter of in_locator object
 */
in_locator
in_locatorKeep(
    in_locator _this);

/** \brief Convert the locator to sockaddr format for raw socket-operations
 */
void
in_locatorToSockaddr(
    in_locator _this,
    struct sockaddr *dest);


/** \brief Convert the locator to sockaddr format, for any address.
 *
 * Leave the address blank,
 * so that bind operation will listen on a port for any address
 */
void
in_locatorToSockaddrForAnyAddress(
		in_locator _this,
		struct sockaddr *dest);


os_int32
in_locatorGetKind(
    in_locator _this);

os_uint32
in_locatorGetPort(
    in_locator _this);

in_address
in_locatorGetIp(
    in_locator _this);

void
in_locatorSetPort(
    in_locator _this,
    os_uint32 port);

c_equality
in_locatorCompare(in_locator _this, in_locator other);

/** \brief serialize the locator */
in_long
in_locatorSerialize(in_locator _this, in_ddsiSerializer _cdr);

/** \brief Private init operation
 *
 * \param port value in range 0..(2^16-1)
 * \param address NULL indicates INADDR_ANY, or corresponding IN6ADDR_ANY_INIT
 *
 * Note: is public to be embedded into in_socket structure */
void
in_locatorInit(in_locator _this,
		os_uint32 port,
		in_address address);

/** \brief public deinit function */
void
in_locatorDeinit(in_locator _this);

/** \brief compare both locators  */
os_boolean
in_locatorEqual(in_locator _this, in_locator other);

/** \brief copy content, does not modify the ref-counters */
void
in_locatorCopy(in_locator _this,
		in_locator copyFrom);


/** clone the object */
in_locator
in_locatorClone(in_locator _this);

/** \brief Verifying locator */
os_boolean
in_locatorIsValid(in_locator _this);

/** \brief Private copy operation,  ref-counter is not modified.
 *
 * Parses the buffer and copies the values into object.
 *
 * \param _this the object
 * \param reader the CDR reader on buffer
 *
 * \return the number of octets read */
in_long
in_locatorCopyFromBuffer(in_locator _this, in_ddsiDeserializer reader);

#endif /* IN_LOCATOR_H_ */
