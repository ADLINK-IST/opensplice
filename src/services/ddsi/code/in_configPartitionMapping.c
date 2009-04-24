#include "in__configPartitionMapping.h"

#include "os_heap.h"

static os_boolean
in_configPartitionMappingInit(
    in_configPartitionMapping _this,
    os_char* nwPartition,
    os_char* partitionTopicCombo);

OS_STRUCT(in_configPartitionMapping)
{
    os_char* dcpsPartitionTopic;
    os_char* nwPartition;
    os_char* pathName;
};

in_configPartitionMapping
in_configPartitionMappingNew(
    os_char* nwPartition,
    os_char* partitionTopicCombo)
{
    in_configPartitionMapping _this;
    os_boolean success;

    assert(nwPartition);
    assert(partitionTopicCombo);

    _this = os_malloc(sizeof(OS_STRUCT(in_configPartitionMapping)));
    if(_this)
    {
        success = in_configPartitionMappingInit(
            _this,
            nwPartition,
            partitionTopicCombo);
        if(!success)
        {
            os_free(_this);
            _this = NULL;
        }
    }

    return _this;
}

os_boolean
in_configPartitionMappingInit(
    in_configPartitionMapping _this,
    os_char* nwPartition,
    os_char* partitionTopicCombo)
{
    os_boolean success = OS_TRUE;

    assert(_this);
    assert(partitionTopicCombo);

    _this->dcpsPartitionTopic = os_strdup(partitionTopicCombo);
    if(!_this->dcpsPartitionTopic)
    {
        success = OS_FALSE;
    }
    if(success)
    {
        _this->nwPartition = os_strdup(nwPartition);
    }
    _this->pathName = NULL;

    return success;
}

os_char*
in_configPartitionMappingGetDcpsPartitionTopic(
    in_configPartitionMapping _this)
{
    assert(_this);

    return _this->dcpsPartitionTopic;
}

os_char*
in_configPartitionMappingGetPathName(
    in_configPartitionMapping _this)
{
    assert(_this);

    return _this->pathName;
}

os_char*
in_configPartitionMappingGetNetworkPartitionName(
    in_configPartitionMapping _this)
{
    assert(_this);

    return _this->nwPartition;
}
