/**
 * A C file holds the implementation of a class. Each class must respectively
 * provide the following:
 *  1)  Documentation of the class:
 *      a) Purpose of the class.
 *      b) Global design of the class with description of all members.
 *
 *  2) Include statements.
 *
 *  3) Declaration and definition of internal private structures (for instance
 *     structures used to pass on arguments of call-back routines).
 *
 *  4)  Potentially the declaration and/or the definition and/or casting macro
 *      of the class (depending on the scope of the class and its members).
 *      a) Declaration:
 *          OS_CLASS(<prefix>_<className>);
 *
 *      b) Definition:
 *          OS_STRUCT(<prefix>_<className>)
 *          {
 *              <members>
 *          };
 *
 *      c) Casting macro:
 *          #define <prefix>_<className>(_this) ((<prefix>_<className>)(_this))
 *
 *          The definition of this macro allows additional functionality to be
 *          implemented when casting (for instance runtime type checking in the
 *          debug environment by redefining the macro with an assertion to force
 *          correctness of the type of the object).
 *
 *      d) Validity check macro:
 *          #define <prefix>_<className>IsValid(_this)( \
 *              in_objectIsValid(in_object(_this)))
 *
 *      e) Increase reference count macro:
 *          #define <prefix>_<className>Keep(_this) (<prefix>_<className>(  \
 *              in_objectKeep(in_object(_this))))
 *
 *  5)  Declaration of static functions.
 *
 *  6)  A constructor:   <prefix>_<className>New (i.e. in_carNew).
 *      The constructor must only allocate the object and call the
 *      initializer method. Abstract classes do NOT have a constructor, but
 *      they do have a destructor.
 *
 *  7)  An initializer:  <prefix>_<className>Init (i.e. in_carInit)
 *      The initializer is responsible to initialize the created object.
 *      Because the initialization is split from the allocation, it is possible
 *      to create classes that extend from the object and allow them to
 *      initialize all members of the super class correctly. The initializer
 *      must always start by calling the initializer of its parent class.
 *
 *      In case the initialization fails, all created resources must be cleaned
 *      up by this function itself. First of all the resources for members of
 *      this class and second also the resources of the parent. The second part
 *      must be done by calling the parent deinitializer.
 *
 *      The first three parameters are the same for all initializers:
 *          a) The object itself. The parameter must always be called '_this'.
 *          b) The object kind.
 *          c) A pointer to the object 'deinitializer' function.
 *
 *  8)  A deinitializer: <prefix>_<className>Deinit (i.e. in_carDeinit)
 *      The deinitializer is responsible to destroy all resources created
 *      as part of the object. The deinitializer is called by the free function
 *      of the base object when the reference count reaches 0. After all
 *      resources of the object members have been freed, the deinitializer of
 *      the parent class must be called. The object itself must NOT be freed
 *      in the deinitializer since the base object takes care of that.
 *
 *  9)  A destructor:    <prefix>_<className>Free (i.e. in_carFree)
 *      The destructor is responsible for calling the destructor of the
 *      parent class. This is the function that must be called by a user of the
 *      class when it no longer needs the particular instance of the class.
 *
 *  The scope of these functions can be limited. Initializers and deinitializers
 *  can be made 'static' functions (private), to indicate that no descendants
 *  can be implemented for this class (aka a final class). The constructor and
 *  destructor scope must be equal to the scope of the class declaration.
 *
 *  10) Getter and setter functions for members (grouped per member).
 *
 *  11) All functions of the class with a 'public' scope.
 *
 *  12) All functions of the class with a 'protected/package' scope
 *
 *  13) All functions of the class with a 'private' scope (so the static ones)
 *
 *  All functions must be documented in a way that doxygen can handle the
 *  documentation. The Javadoc-style using '@' to indicate keywords must be
 *  used, because the Java (using Javadoc) and C documentation can both be
 *  generated with the same Doxygen configuration. The documentation for a
 *  function must be written at the location it is declared. This means that
 *  documentation can also be in a header file (depending on the scope of the
 *  function). The documentation must contain:
 *  a) Description of what the function does, ideally with pre- and
 *     postconditions.
 *  b) Description of each parameters (using '@param').
 *  c) Description of the possible return values with their conditions
 *     (using '@return').
 */

#include "in__object.h"
#include "os_abstract.h"
#include "os_heap.h"

#define IN_OBJECT_CONFIDENCE        ((os_uint32)0x4E614D65u) /* 'NaMe' */
#define IN_OBJECT_CONFIDENCE_NULL   ((os_uint32)0x000000000)

static char* in_kindString[] = {
	"IN_OBJECT_KIND_INVALID                    ",
	"IN_OBJECT_KIND_LOCATOR                    ",
	"IN_OBJECT_KIND_LOCATOR_EMBEDDED           ",
	"IN_OBJECT_KIND_MESSAGE_DESERIALIZER       ",
	"IN_OBJECT_KIND_MESSAGE_SERIALIZER         ",
	"IN_OBJECT_KIND_PEER_READER                ",
	"IN_OBJECT_KIND_WRITER_FACADE              ",
	"IN_OBJECT_KIND_READER_FACADE              ",
	"IN_OBJECT_KIND_RECEIVE_CHANNEL            ",
	"IN_OBJECT_KIND_SEND_CHANNEL               ",
	"IN_OBJECT_KIND_DATA_BUFFER_BASIC          ",
	"IN_OBJECT_KIND_SEND_BUFFER_BASIC          ",
	"IN_OBJECT_KIND_RECEIVE_BUFFER_BASIC       ",
	"IN_OBJECT_KIND_TRANSPORT_RECEIVER_BASIC   ",
	"IN_OBJECT_KIND_TRANSPORT_SENDER_BASIC     ",
	"IN_OBJECT_KIND_TRANSPORT_PAIR_BASIC       ",
	"IN_OBJECT_KIND_STREAM_WRITER_BASIC        ",
	"IN_OBJECT_KIND_STREAM_READER_BASIC        ",
	"IN_OBJECT_KIND_STREAM_PAIR_BASIC          ",
	"IN_OBJECT_KIND_CONNECTIVITY_ADMIN         ",
	"IN_OBJECT_KIND_PARTICIPANT_FACADE         ",
	"IN_OBJECT_KIND_PEER_PARTICIPANT           ",
	"IN_OBJECT_KIND_PEER_WRITER                ",
	"IN_OBJECT_KIND_DATA_CHANNEL               ",
	"IN_OBJECT_KIND_DATA_CHANNEL_WRITER        ",
	"IN_OBJECT_KIND_DATA_CHANNEL_READER        ",
	"IN_OBJECT_KIND_SDP_CHANNEL                ",
	"IN_OBJECT_KIND_SDP_WRITER                 ",
	"IN_OBJECT_KIND_SDP_READER                 ",
	"IN_OBJECT_KIND_PLUG_KERNEL                ",
	"IN_OBJECT_KIND_DISCOVERED_PARTICIPANT_DATA",
	"IN_OBJECT_KIND_DISCOVERED_WRITER_DATA     ",
	"IN_OBJECT_KIND_DISCOVERED_READER_DATA     ",
	"IN_OBJECT_KIND_ENDPOINT_DISCOVERY_DATA    ",
	"IN_OBJECT_KIND_SOCKET                     ",
	"IN_OBJECT_KIND_COUNT                      "
};

static c_ulong allocationCount = 0;
static c_ulong maxObjectCount = 0;
static c_ulong typedObjectCount[IN_OBJECT_KIND_COUNT];
static c_ulong maxTypedObjectCount[IN_OBJECT_KIND_COUNT];

static c_bool
doAdd(
	in_objectKind kind);

static c_bool
doSub(
	in_objectKind kind);

os_boolean
in_objectInit(
    in_object _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit)
{
    assert(_this);
    assert(kind > IN_OBJECT_KIND_INVALID);
    assert(kind < IN_OBJECT_KIND_COUNT);
    assert(deinit);

    _this->confidence = IN_OBJECT_CONFIDENCE;
    _this->kind = kind;
    _this->deinit = deinit;
    _this->refCount = 1;

    assert(doAdd(kind));

    return OS_TRUE;
}

void
in_objectFree(
    in_object _this)
{
    os_uint32 refCount;

    /* Check whether the object has been allocated to support calling this
     * function with a NULL pointer.
     */
    if(_this)
    {
        assert(_this->confidence == IN_OBJECT_CONFIDENCE);
        assert(_this->refCount >= 1);

        refCount = pa_decrement(&(_this->refCount));
/*
        if(_this->kind == IN_OBJECT_KIND_PARTICIPANT_FACADE)
		{
			printf("Free %p: %d -> %d\n", _this, _this->refCount+1, _this->refCount);
		}
*/
        if(refCount == 0)
        {
            if(_this->deinit)
            {
                _this->deinit(_this);
            }
            assert(doSub(_this->kind));
            _this->confidence = IN_OBJECT_CONFIDENCE_NULL;
            _this->kind = IN_OBJECT_KIND_INVALID;
            os_free(_this);
        }
    }
}

in_objectKind
in_objectGetKind(
    in_object _this)
{
    assert(_this);
    assert(_this->confidence == IN_OBJECT_CONFIDENCE);

    return _this->kind;
}

os_uint32
in_objectGetRefCount(
    in_object _this)
{
    assert(_this);
    assert(_this->confidence == IN_OBJECT_CONFIDENCE);

    return _this->refCount;
}

os_boolean
in_objectIsValid(
    in_object _this)
{
    os_boolean result;

    if(_this)
    {
        if(_this->confidence == IN_OBJECT_CONFIDENCE)
        {
            if((_this->kind > IN_OBJECT_KIND_INVALID) &&
               (_this->kind < IN_OBJECT_KIND_COUNT))
            {
                result = OS_TRUE;
            } else
            {
                result = OS_FALSE;
            }
        } else
        {
            result = OS_FALSE;
        }
    } else
    {
        result = OS_FALSE;
    }
    return result;
}

os_boolean
in_objectIsValidWithKind(
    in_object _this,
    in_objectKind kind)
{
    os_boolean result;

    result = in_objectIsValid(_this);

    if(result)
    {
        if(_this->kind != kind)
        {
            result = OS_FALSE;
        }
    }
    return result;
}

in_object
in_objectKeep(
    in_object _this)
{
    in_object result;

    assert(_this);
    assert(_this->confidence == IN_OBJECT_CONFIDENCE);
    assert(_this->refCount >= 1);

    /* Check whether the object is allocated to support a keep on a NULL
     * pointer.
     */
    if(_this)
    {
        pa_increment(&(_this->refCount));
/*
        if(_this->kind == IN_OBJECT_KIND_PARTICIPANT_FACADE)
        {
        	printf("Keep %p: %d -> %d\n", _this, _this->refCount-1, _this->refCount);
        }
*/
        result = _this;
    } else
    {
        result = NULL;
    }
    return result;
}

#ifndef NDEBUG
in_object
in_objectFromObjectRef(
    in_objectRef _this)
{
    in_object object;

    /* Check whether the object is allocated to support a in_objectFromObjectRef
     * on a NULL pointer.
     */
    if(_this)
    {
        object = (in_object)_this;
        assert(in_objectIsValid(object));
    } else
    {
        /* This is a valid situation and is therefore allowed. */
        object = NULL;
    }
    return object;
}

in_objectRef
in_objectRefFromObject(
    in_object _this)
{
    in_objectRef ref;

    /* Check whether the object is allocated to support a in_objectRefFromObject
     * on a NULL pointer.
     */
    if(_this)
    {
        assert(in_objectIsValid(_this));
        ref = (in_objectRef)_this;
    } else
    {
        /* This is a valid situation and is therefore allowed. */
        ref = NULL;
    }
    return ref;
}
#endif

c_bool
in_objectValidate(
    c_ulong expected)
{
    c_bool result;
    c_ulong i;

    printf("\nHeap allocation report:\n");
    printf("-------------------------------------------------\n");
    printf("Type\t\t\t\t\t\tCurrent\tTotal\n");
    printf("-------------------------------------------------\n");

    for(i=1; i<IN_OBJECT_KIND_COUNT; i++){
    	/*Not counting IN_OBJECT_KIND_INVALID and IN_OBJECT_KIND_COUNT*/
        printf("%s\t%d\t%d\n", in_kindString[i], typedObjectCount[i], maxTypedObjectCount[i]);
    }
    printf("-------------------------------------------------\n");
    printf("\n#allocated: %d, #remaining: %d, #expected: %d\n",
            maxObjectCount, allocationCount, expected);

    if(expected != allocationCount){
        printf("Allocation validation [ FAILED ]\n");
        result = FALSE;
    } else {
        printf("Allocation validation [   OK   ]\n");
        result = TRUE;
    }

    return TRUE;
}

static c_bool
doAdd(
	in_objectKind kind)
{
    c_ulong i;
    c_long add;

    add = pa_increment(&maxObjectCount);

    if(add == 1){
        for(i=0; i<IN_OBJECT_KIND_COUNT; i++){
            typedObjectCount[i] = 0;
            maxTypedObjectCount[i] = 0;
        }
    }
    pa_increment(&allocationCount);
    pa_increment(&(typedObjectCount[kind]));
    pa_increment(&(maxTypedObjectCount[kind]));

    return TRUE;
}

static c_bool
doSub(
	in_objectKind kind)
{
    pa_decrement(&allocationCount);
    pa_decrement(&(typedObjectCount[kind]));

    return TRUE;
}
