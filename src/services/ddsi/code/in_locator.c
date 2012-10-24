/* OS abstraction includes */
#include "os_heap.h"

/* DDSi includes */
#include <assert.h>
#include "in_ddsiElements.h"
#include "in__locator.h"
#include "in_address.h"

/* -------------------------------------------------- */
/* ---- in_locator ---------------------------------- */
/* -------------------------------------------------- */


/** \brief private constructor invoked by subclass-constructors */
static void
in_locatorInitParent(
	in_locator _this,
	in_objectKind objKind,
	in_objectDeinitFunc destroy,
	os_uint32 port,
	in_address address);

/* ------- private definitions ------- */


in_long
in_locatorCopyFromBuffer(
	in_locator _this,
	in_ddsiDeserializer reader)
{
	/* Note: you must not modify the ref-counter!! This operation is invoked
	 * by EmbeddedPool to set state of the pre-allocated object  */

	/* replace current values by new ones parsed from buffer */
	in_long result = in_ddsiLocatorInitFromBuffer(&(_this->value), reader);

	return result;
}

static void
in_locatorInitParent(in_locator _this,
	in_objectKind objKind,
	in_objectDeinitFunc destroy,
	os_uint32 port,
	in_address address)
{
	os_int32 kind = in_addressIsIPv4Compatible(address)? IN_LOCATOR_KIND_UDPV4
		: IN_LOCATOR_KIND_UDPV6;

	in_objectInit(OS_SUPER(_this), objKind, destroy);
	in_ddsiLocatorInit(&(_this->value), kind, port, address);
}

/* ------- public operations ------- */

in_locator
in_locatorNewFromString(
	const os_char* addressString,
	os_uint32 port)
{
	in_locator result = NULL;
	/* transform to sockaddr_in or sockaddr_in6 */
	OS_STRUCT(in_address) tmpAddress;

	if (in_addressInitFromString(&tmpAddress, addressString))
	{
	    result = in_locatorNew(port, &tmpAddress);
	}

    return result;
}

in_locator
in_locatorNew(
	os_uint32 port,
	in_address address)
{
	in_locator result = in_locator(os_malloc(sizeof(*result)));

	if (result)
	{
		/* the type can be derived from address */
		in_locatorInit(result, port, address);
	}
	return result;
}

void
in_locatorInitInvalid(
	in_locator _this)
{
	in_objectInit(OS_SUPER(_this), IN_OBJECT_KIND_LOCATOR,
		(in_objectDeinitFunc) in_locatorDeinit);
	in_ddsiLocatorInitInvalid(&(_this->value));
}


void
in_locatorInitFromSockaddr(
	in_locator _this,
	const struct sockaddr *sockAddr)
{
	assert(_this);

	in_ddsiLocatorInitFromSockaddr(&(_this->value), sockAddr);
}

void
in_locatorInitUDPv4(
	in_locator _this,
	os_uint32 port,
	const in_addressIPv4 address)
{
	assert(_this);

	in_objectInit(OS_SUPER(_this), IN_OBJECT_KIND_LOCATOR,
		(in_objectDeinitFunc) in_locatorDeinit);
	in_ddsiLocatorInitUDPv4(&(_this->value), port, address);
}


void
in_locatorDeinit(
    in_locator _this)
{
    assert(_this);

	in_ddsiLocatorDeinit(&(_this->value));
    in_objectDeinit(OS_SUPER(_this));
}

void
in_locatorInitUDPv6(in_locator _this,
	os_uint32 port,
	const in_addressIPv6 address)
{
	assert(_this);

	in_objectInit(OS_SUPER(_this), IN_OBJECT_KIND_LOCATOR,
		(in_objectDeinitFunc) in_locatorDeinit);
	in_ddsiLocatorInitUDPv6(&(_this->value), port, address);
}

void
in_locatorFree(
	in_locator _this)
{
	assert(_this);

	/* decrement refcount */
	in_objectFree(in_object(_this));
}

/* increment refcounter */
in_locator
in_locatorKeep(in_locator _this)
{
	assert(_this);
	return in_locator(in_objectKeep(in_object(_this)));

}

void
in_locatorToSockaddr(in_locator _this, struct sockaddr *dest)
{
	assert(_this);
	assert(dest);

	in_ddsiLocatorToSockaddr(&(_this->value), dest);
}

void
in_locatorToSockaddrForAnyAddress(in_locator _this, struct sockaddr *dest)
{
	assert(_this);
	assert(dest);

	in_ddsiLocatorToSockaddrForAnyAddress(&(_this->value), dest);
}

os_int32
in_locatorGetKind(in_locator _this)
{
	assert(_this);

	return in_ddsiLocatorGetKind(&(_this->value));
}

os_uint32
in_locatorGetPort(in_locator _this)
{
	assert(_this);

	return in_ddsiLocatorGetPort(&(_this->value));
}

in_address
in_locatorGetIp(in_locator _this)
{
	assert(_this);

	return in_ddsiLocatorGetIp(&(_this->value));
}

void
in_locatorSetPort(
	in_locator _this,
	os_uint32 port)
{
	assert(_this);

	in_ddsiLocatorSetPort(&(_this->value), port);
}

c_equality
in_locatorCompare(in_locator _this,
		in_locator other)
{
	return in_ddsiLocatorCompare(&(_this->value), &(other->value));
}

/** \brief serialize the locator
 * Just a wrapper, may be replaced by macro */
in_long
in_locatorSerialize(in_locator _this, in_ddsiSerializer writer)
{
	assert(_this);

	return in_ddsiLocatorSerialize(&(_this->value), writer);
}

/** \brief public init function
 * Note: is public to be embedded into in_socket structure */
void
in_locatorInit(in_locator _this,
		os_uint32 port,
		in_address address)
{
	os_int32 kind = in_addressIsIPv4Compatible(address)?
			IN_LOCATOR_KIND_UDPV4 : IN_LOCATOR_KIND_UDPV6;

	assert(_this);

	in_objectInit(OS_SUPER(_this), IN_OBJECT_KIND_LOCATOR,
		(in_objectDeinitFunc) in_locatorDeinit);
	in_ddsiLocatorInit(&(_this->value), kind, port, address);
}

/** \brief compare both locators  */
os_boolean
in_locatorEqual(in_locator _this, in_locator other)
{
	return in_ddsiLocatorEqual(&(_this->value), &(other->value));
}

/** \brief copy content
 *
 * Does not modify the ref-counter.
 * Note: could be turned into macro */
void
in_locatorCopy(
	in_locator _this,
	in_locator copyFrom)
{
	in_ddsiLocatorCopy(&(_this->value), &(copyFrom->value));
}


/** */
in_locator
in_locatorClone(
	in_locator _this)
{

    in_locator result =
        os_malloc(sizeof(*result));

    if (result)
    {
    	in_locatorInit(result, in_locatorGetPort(_this), in_locatorGetIp(_this));
    }
    return result;
}


/** \brief Verifying locator */
os_boolean
in_locatorIsValid(
	in_locator _this)
{
	os_boolean result;
	result = _this && in_ddsiLocatorIsValid(&(_this->value));
	return result;
}
