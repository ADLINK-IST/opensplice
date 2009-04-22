#include "in__configPartitioning.h"

#include "os_heap.h"

static os_boolean
in_configPartitioningInit(
    in_configPartitioning _this);

OS_STRUCT(in_configPartitioning)
{
    os_char* pathName;
    os_char* globalPartitionAddress;
    Coll_List networkPartitions;
    Coll_List partitionMappings;
};

in_configPartitioning
in_configPartitioningNew(
    )
{
    in_configPartitioning _this;
    os_boolean success;

    _this = os_malloc(sizeof(OS_STRUCT(in_configPartitioning)));
    if(_this)
    {
        success = in_configPartitioningInit(_this);
        if(!success)
        {
            os_free(_this);
            _this = NULL;
        }
    }

    return _this;
}

os_boolean
in_configPartitioningInit(
    in_configPartitioning _this)
{
    assert(_this);

    _this->pathName = NULL;
    _this->globalPartitionAddress = NULL;
    Coll_List_init(&(_this->networkPartitions));
    Coll_List_init(&(_this->partitionMappings));

    return OS_TRUE;
}

void
in_configPartitioningAddNetworkPartition(
    in_configPartitioning _this,
    in_configNetworkPartition networkPartition)
{
    os_uint32 result;

    assert(_this);
    assert(networkPartition);

    result = Coll_List_pushBack(&(_this->networkPartitions), networkPartition);
    if(result != COLL_OK)
    {
        /* TODO report out of memory error */
    }
}

void
in_configPartitioningAddPartitionMapping(
    in_configPartitioning _this,
    in_configPartitionMapping partitionMapping)
{
    os_uint32 result;

    assert(_this);
    assert(partitionMapping);

    result = Coll_List_pushBack(&(_this->partitionMappings), partitionMapping);
    if(result != COLL_OK)
    {
        /* TODO report out of memory error */
    }
}

void
in_configPartitioningSetGlobalPartitionAddress(
    in_configPartitioning _this,
    os_char* address)
{
    assert(_this);
    assert(address);

    if(_this->globalPartitionAddress)
    {
        os_free(_this->globalPartitionAddress);
    }
    _this->globalPartitionAddress = os_strdup(address);
}

os_char*
in_configPartitioningGetGlobalPartitionAddress(
    in_configPartitioning _this)
{
    assert(_this);

    return _this->globalPartitionAddress;
}

/* List contains 'in_configNetworkPartition' objects */
Coll_List*
in_configPartitioningGetNetworkPartitions(
    in_configPartitioning _this)
{
    assert(_this);

    return &(_this->networkPartitions);
}

/* List contains 'in_configPartitionMapping' objects */
Coll_List*
in_configPartitioningGetPartitionsMappings(
    in_configPartitioning _this)
{
    assert(_this);

    return &(_this->partitionMappings);
}

os_char*
in_configPartitioningGetPathName(
    in_configPartitioning _this)
{
    assert(_this);

    return _this->pathName;
}
