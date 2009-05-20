#include "in__configNetworkPartition.h"

#include "os_heap.h"

static os_boolean
in_configNetworkPartitionInit(
    in_configNetworkPartition _this,
    os_char* name,
    os_char* address,
    os_boolean isConnected);

OS_STRUCT(in_configNetworkPartition)
{
    os_char* name;
    os_char* address;
    os_boolean isConnected;
    os_char* pathName;
};

in_configNetworkPartition
in_configNetworkPartitionNew(
    os_char* name,
    os_char* address,
    os_boolean isConnected)
{
    in_configNetworkPartition _this;
    os_boolean success;

    assert(name);

    _this = os_malloc(sizeof(OS_STRUCT(in_configNetworkPartition)));
    if(_this)
    {
        success = in_configNetworkPartitionInit(
            _this,
            name,
            address,
            isConnected);
        if(!success)
        {
            os_free(_this);
            _this = NULL;
        }
    }

    return _this;
}

os_boolean
in_configNetworkPartitionInit(
    in_configNetworkPartition _this,
    os_char* name,
    os_char* address,
    os_boolean isConnected)
{
    os_boolean success = OS_TRUE;

    _this->name = os_strdup(name);
    if(!_this->name)
    {
        success = OS_FALSE;
    }
    if(success)
    {
        _this->address = os_strdup(address);
        if(!_this->address)
        {
            success = OS_FALSE;
        }
    }
    _this->isConnected = isConnected;
    _this->pathName = NULL;

    return success;
}

os_char*
in_configNetworkPartitionGetName(
    in_configNetworkPartition _this)
{
    assert(_this);

    return _this->name;
}

os_char*
in_configNetworkPartitionGetAddress(
    in_configNetworkPartition _this)
{
    assert(_this);

    return _this->address;
}

os_char*
in_configNetworkPartitionGetPathName(
    in_configNetworkPartition _this)
{
    assert(_this);

    return _this->pathName;
}

os_boolean
in_configNetworkPartitionIsConnected(
    in_configNetworkPartition _this)
{
    assert(_this);

    return _this->isConnected;
}
