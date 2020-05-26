#
#                         Vortex OpenSplice
#
#   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
#   Technology Limited, its affiliated companies and licensors. All rights
#   reserved.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
'''
DDS Python DCPS API class

'''
cimport dds
from enum import Enum
from libc.stdlib cimport malloc, free
from libc.string cimport memset
from libc.stdint cimport intptr_t
from cpython.mem cimport PyMem_Malloc, PyMem_Realloc, PyMem_Free
from collections import namedtuple
import inspect
import time

# Required to have listener's successfully call back into Python
PyEval_InitThreads()


#############################################################################
### Special notes on GIL retention when calling C99 APIs
#
# The Python GIL (Global Interface Lock) is a lock that must be held when
# a thread is executing Python code. Only one thread at a time may hold the
# GIL.
#
# Any DDS C99 API that does any of the following should relinquish the GIL:
#  * performs a long running operation (wait, file I/O, etc)
#  * may trigger a Listener (which necessarily runs on another thread)
#  * may acquire a DDS lock, with a resulting wait before lock acquisition
#
# The first instinct is to run all C99 APIs without the GIL, but this may
# have a performance impact (releasing and acquiring the GIL is 'expensive').
#
# As an alternative, all the C99 functions called here are divided into one
# of three categories
#  1) 'gil_ok' - not long running, does not acquire locks in DDS itself.
# These are typically functions the 'allocate' or free resources'. Generally,
# the C99 function calls an SAC function that ends in Alloc or Free, or is
# merely copying (perhaps with allocation) of data. Such functions are marked
# herein with a comment: # gil_ok
#
#  2) 'nogil required' - generally, C99 methods that do not meet the 'gil_ok'
# standard must be executed within a 'with nogil' statement. Such methods
# are easily observed because of the wrapping 'with nogil:' statement.
#
#  3) 'needs_gil' - exceptions to #2, where a lock is required, because
# two or more C99 methods must be executed in single indivisible action
# in order to avoid race conditions. At this point, this only happens
# during entity create, where a listener is registered. Successful
# registration of a Python Listener requires three steps:
#    i) calling the dds_xxx_create(..., listener, ...) method
#    ii) calling _entity_register() to insert the handle into the global
#       table of handles and Python Entity objects
#    iii) calling _set_status_for_listener() to correctly configure the
#       C99 entity's 'status' to reflect the registered listener methods
# In such situations, the C99 method is marked with the comment: # needs_gil
# The approach taken to continue to hold the GIL during the above three
# sub-steps, and then, once completed, yield to potentially triggered
# listeners (including the newly registered one) by calling time.sleep(0)

#############################################################################
### Sample Info States
class DDSStateKind(Enum):
    READ_SAMPLE_STATE = 1
    NOT_READ_SAMPLE_STATE = 2
    ANY_SAMPLE_STATE = READ_SAMPLE_STATE | NOT_READ_SAMPLE_STATE
    NEW_VIEW_STATE = 4
    NOT_NEW_VIEW_STATE = 8
    ANY_VIEW_STATE = NEW_VIEW_STATE | NOT_NEW_VIEW_STATE
    ALIVE_INSTANCE_STATE = 16
    NOT_ALIVE_DISPOSED_INSTANCE_STATE = 32
    NOT_ALIVE_NO_WRITERS_INSTANCE_STATE = 64
    ANY_INSTANCE_STATE = ALIVE_INSTANCE_STATE | NOT_ALIVE_DISPOSED_INSTANCE_STATE | NOT_ALIVE_NO_WRITERS_INSTANCE_STATE
    ANY_STATE = ANY_SAMPLE_STATE | ANY_VIEW_STATE | ANY_INSTANCE_STATE

#############################################################################
### Communication status
class DDSStatusKind(Enum):
    INCONSISTENT_TOPIC         = dds.DDS_INCONSISTENT_TOPIC_STATUS
    OFFERED_DEADLINE_MISSED    = dds.DDS_OFFERED_DEADLINE_MISSED_STATUS
    REQUESTED_DEADLINE_MISSED  = dds.DDS_REQUESTED_DEADLINE_MISSED_STATUS
    OFFERED_INCOMPATIBLE_QOS   = dds.DDS_OFFERED_INCOMPATIBLE_QOS_STATUS
    REQUESTED_INCOMPATIBLE_QOS = dds.DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS
    SAMPLE_LOST                = dds.DDS_SAMPLE_LOST_STATUS
    SAMPLE_REJECTED            = dds.DDS_SAMPLE_REJECTED_STATUS
    DATA_ON_READERS            = dds.DDS_DATA_ON_READERS_STATUS
    DATA_AVAILABLE             = dds.DDS_DATA_AVAILABLE_STATUS
    LIVELINESS_LOST            = dds.dds.DDS_LIVELINESS_LOST_STATUS
    LIVELINESS_CHANGED         = dds.DDS_LIVELINESS_CHANGED_STATUS
    PUBLICATION_MATCHED        = dds.DDS_PUBLICATION_MATCHED_STATUS
    SUBSCRIPTION_MATCHED       = dds.DDS_SUBSCRIPTION_MATCHED_STATUS

#############################################################################
## QoS IDs
class QosPolicyId(Enum):
    '''
    Enumeration of Qos policy kinds
    '''
    INVALID_QOS_POLICY_ID = 0
    USERDATA_QOS_POLICY_ID = 1
    DURABILITY_QOS_POLICY_ID = 2
    PRESENTATION_QOS_POLICY_ID = 3
    DEADLINE_QOS_POLICY_ID = 4
    LATENCYBUDGET_QOS_POLICY_ID = 5
    OWNERSHIP_QOS_POLICY_ID = 6
    OWNERSHIPSTRENGTH_QOS_POLICY_ID = 7
    LIVELINESS_QOS_POLICY_ID = 8
    TIMEBASEDFILTER_QOS_POLICY_ID = 9
    PARTITION_QOS_POLICY_ID = 10
    RELIABILITY_QOS_POLICY_ID = 11
    DESTINATIONORDER_QOS_POLICY_ID = 12
    HISTORY_QOS_POLICY_ID = 13
    RESOURCELIMITS_QOS_POLICY_ID = 14
    ENTITYFACTORY_QOS_POLICY_ID = 15
    WRITERDATALIFECYCLE_QOS_POLICY_ID = 16
    READERDATALIFECYCLE_QOS_POLICY_ID = 17

    TOPICDATA_QOS_POLICY_ID = 18
    GROUPDATA_QOS_POLICY_ID = 19
    TRANSPORTPRIORITY_QOS_POLICY_ID = 20
    LIFESPAN_QOS_POLICY_ID = 21
    DURABILITYSERVICE_QOS_POLICY_ID = 22

#############################################################################
#### QoS Kinds

class DDSTime:
    '''
    DDS Time

    :type sec: long
    :param sec: seconds (default: 1)

    :type nanosec: long
    :param nanosec: nanoseconds (default: 0)
    '''
    def __init__(self, sec = 1, nanosec = 0):
        self.value = (sec * 1000000000) + nanosec

class DDSDuration(DDSTime):
    '''
    DDS Duration

    :type sec: long
    :param sec: seconds (default: 0)

    :type nanosec: long
    :param nanosec: nanoseconds (default: 0)
    '''

    @staticmethod
    def infinity():
        ''' Return DDSDuration for infinity
        '''
        return DDSDuration(0, DDS_INFINITY)

### Durability
class DDSDurabilityKind(Enum):
    VOLATILE = 0
    TRANSIENT_LOCAL = 1
    TRANSIENT = 2
    PERSISTENT = 3

### History
class DDSHistoryKind(Enum):
    KEEP_LAST = 0
    KEEP_ALL = 1

class DDSPresentationAccessScopeKind(Enum):
    INSTANCE = 0
    TOPIC = 1
    GROUP = 2

### Ownership
class DDSOwnershipKind(Enum):
    SHARED = 0
    EXCLUSIVE = 1

### Reliability
class DDSReliabilityKind(Enum):
    BEST_EFFORT = 0
    RELIABLE = 1

### Dest Order
class DDSDestinationOrderKind(Enum):
    BY_RECEPTION_TIMESTAMP = 0
    BY_SOURCE_TIMESTAMP = 1

### Liveliness kind
class DDSLivelinessKind(Enum):
    AUTOMATIC = 0
    MANUAL_BY_PARTICIPANT = 1
    MANUAL_BY_TOPIC = 2

### DDSSampleRejectedStatusKind
class DDSSampleRejectedStatusKind(Enum):
    NOT_REJECTED = 0
    REJECTED_BY_INSTANCE_LIMIT = 1
    REJECTED_BY_SAMPLES_LIMIT = 2
    REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT = 3

# Basic Qos Policy class
class QosPolicy():
    '''
    Abstract QoS policy class.
    Users should not instantiate this class.
    '''
    def __init__(self, i):
        '''__init__(i)
        '''
        self._kind = 0
        self._id = i

    @property
    def id(self):
        '''
        An internal QoS Policy id
        '''
        return self._id

    # The kind property provides the kind of a specific QosPolicy when applicable
    @property
    def kind(self):
        '''
        QoS Policy kind
        '''
        return self._kind

class DurabilityQosPolicy(QosPolicy):
    '''
    Durability QoS Policy

    :type k: DDSDurabilityKind
    :param k: Durability kind
    '''

    def __init__(self, kind=DDSDurabilityKind.VOLATILE):
        '''__init__(k=DDSDurabilityKind.VOLATILE)
        '''
        QosPolicy.__init__(self, QosPolicyId.DURABILITY_QOS_POLICY_ID)
        self._kind = kind


class HistoryQosPolicy(QosPolicy):
    '''
    History QoS Policy

    :type k: DDSHistoryKind
    :param k: History kind

    :type depth: int
    :param depth: History depth (default: 1)
    '''
    def __init__(self, kind=DDSHistoryKind.KEEP_LAST, depth=1):
        '''__init__(kind=DDSHistoryKind.KEEP_LAST, depth=1)
        '''
        QosPolicy.__init__(self, QosPolicyId.HISTORY_QOS_POLICY_ID)
        self._kind = kind
        self._depth = depth

    @property
    def depth(self):
        return self._depth

class ResourceLimitsQosPolicy(QosPolicy):
    '''
    Resource limit QoS Policy

    :type max_samples: int
    :param max_samples: maximum samples (default: -1)

    :type max_instances: int
    :param max_instances: maximum instances (default: -1)

    :type max_samples_per_instance: int
    :param max_samples_per_instance: maximum samples per instance (default: -1)
    '''
    def __init__(self, max_samples = DDS_LENGTH_UNLIMITED, max_instances = DDS_LENGTH_UNLIMITED, max_samples_per_instance = DDS_LENGTH_UNLIMITED):
        QosPolicy.__init__(self, QosPolicyId.RESOURCELIMITS_QOS_POLICY_ID)
        self._max_samples = max_samples
        self._max_instances = max_instances
        self._max_samples_per_instance = max_samples_per_instance

    @property
    def max_samples(self):
        return self._max_samples

    @property
    def max_instances(self):
        return self._max_instances

    @property
    def max_samples_per_instance(self):
        return self._max_samples_per_instance

class PresentationQosPolicy(QosPolicy):
    '''
    Presentation QoS Policy

    :type kind: DDSPresentationAccessScopeKind
    :param kind: Presentation kind (default: INSTANCE)

    :type coherent_access: bool
    :param coherent_access: Set coherent access (default: False)

    :type ordered_access: bool
    :param ordered_access: Set ordered access (default: False)
    '''
    def __init__(self, kind=DDSPresentationAccessScopeKind.INSTANCE, coherent_access = False, ordered_access = False):
        '''__init__(kind=DDSHistoryKind.KEEP_LAST, depth=1)
        '''
        QosPolicy.__init__(self, QosPolicyId.PRESENTATION_QOS_POLICY_ID)
        self._kind = kind
        self._coherent_access = coherent_access
        self._ordered_access = ordered_access

    @property
    def coherent_access(self):
        return self._coherent_access

    @property
    def ordered_access(self):
        return self._ordered_access

class LifespanQosPolicy(QosPolicy):
    '''
    Lifespan QoS Policy

    :type lifespan: DDSDuration
    :param lifespan: Value for lifespan (default: infinity)

    '''
    def __init__(self, lifespan = DDSDuration.infinity()):
        QosPolicy.__init__(self, QosPolicyId.LIFESPAN_QOS_POLICY_ID)
        self._lifespan = lifespan.value

    @property
    def lifespan(self):
        return self._lifespan

class DeadlineQosPolicy(QosPolicy):
    '''
    Deadline QoS Policy

    :type deadline: DDSDuration
    :param deadline: Value for deadline (default: infinity)

    '''
    def __init__(self, deadline = DDSDuration.infinity()):
        QosPolicy.__init__(self, QosPolicyId.DEADLINE_QOS_POLICY_ID)
        self._deadline = deadline.value

    @property
    def deadline(self):
        return self._deadline

class LatencyBudgetQosPolicy(QosPolicy):
    '''
    Latency budget QoS Policy

    :type duration: DDSDuration
    :param duration: Value for duration (default: 0)

    '''
    def __init__(self, duration = DDSDuration()):
        QosPolicy.__init__(self, QosPolicyId.LATENCYBUDGET_QOS_POLICY_ID)
        self._duration = duration.value

    @property
    def duration(self):
        return self._duration

class OwnershipQosPolicy(QosPolicy):
    '''
    Ownership QoS Policy

    :type kind: DDSOwnershipKind
    :param kind: Ownership kind

    '''
    def __init__(self, kind=DDSOwnershipKind.SHARED):
        QosPolicy.__init__(self, QosPolicyId.OWNERSHIP_QOS_POLICY_ID)
        self._kind = kind

class OwnershipStrengthQosPolicy(QosPolicy):
    '''
    Ownership strenth QoS Policy

    :type value: int
    :param value: value for strength (default: 0)

    '''
    def __init__(self, value = 0):
        QosPolicy.__init__(self, QosPolicyId.OWNERSHIPSTRENGTH_QOS_POLICY_ID)
        self._value = value

    @property
    def value(self):
        return self._value


class LivelinessQosPolicy(QosPolicy):
    '''
    Liveliness QoS Policy

    :type kind: DDSLivelinessKind
    :param kind: Liveliness kind

    :type lease_duration: DDSDuration
    :param lease_duration: Lease duration (default: infinity)
    '''
    def __init__(self, kind=DDSLivelinessKind.AUTOMATIC, lease_duration=DDSDuration.infinity()):
        QosPolicy.__init__(self, QosPolicyId.LIVELINESS_QOS_POLICY_ID)
        self._kind = kind
        self._lease = lease_duration.value

    @property
    def lease_duration(self):
        return self._lease

class TimeBasedFilterQosPolicy(QosPolicy):
    '''
    Time based filter QoS Policy

    :type minimum_separation: DDSDuration
    :param minimum_separation: Value for minimum separation (default: 0)

    '''
    def __init__(self, minimum_separation = DDSDuration(0,0)):
        QosPolicy.__init__(self, QosPolicyId.TIMEBASEDFILTER_QOS_POLICY_ID)
        self._minimum_separation = minimum_separation.value

    @property
    def minimum_separation(self):
        return self._minimum_separation

class PartitionQosPolicy(QosPolicy):
    '''
    Partition QoS Policy

    :type ps: list
    :param ps: list of partition names

    '''
    def __init__(self, ps = []):
        QosPolicy.__init__(self, QosPolicyId.PARTITION_QOS_POLICY_ID)
        self._names = ps

    @property
    def ps(self):
        return self._names

class ReliabilityQosPolicy(QosPolicy):
    '''
    Reliability QoS Policy

    :type kind: DDSReliabilityKind
    :param kind: Reliability kind

    :type max_blocking_time: DDSDuration
    :param max_blocking_time: Max blocking time (default: 100 milliseconds)
    '''
    def __init__(self, kind=DDSReliabilityKind.BEST_EFFORT, max_blocking_time=DDSDuration(0,100000000)):
        QosPolicy.__init__(self, QosPolicyId.RELIABILITY_QOS_POLICY_ID)
        self._kind = kind
        self._max_blocking_time = max_blocking_time.value

    @property
    def max_blocking_time(self):
        return self._max_blocking_time

class TransportPriorityQosPolicy(QosPolicy):
    '''
    Transport priority QoS Policy

    :type value: int
    :param value: value for transport priority

    '''
    def __init__(self, value = 0):
        QosPolicy.__init__(self, QosPolicyId.TRANSPORTPRIORITY_QOS_POLICY_ID)
        self._value = value

    @property
    def value(self):
        return self._value

class DestinationOrderQosPolicy(QosPolicy):
    '''
    Destination order QoS Policy

    :type kind: DDSDestinationOrderKind
    :param kind: Destination order kind

    '''
    def __init__(self, kind=DDSDestinationOrderKind.BY_RECEPTION_TIMESTAMP):
        QosPolicy.__init__(self, QosPolicyId.DESTINATIONORDER_QOS_POLICY_ID)
        self._kind = kind

class WriterDataLifecycleQosPolicy(QosPolicy):
    '''
    Writer data lifecycle QoS Policy

    :type autodispose_unregistered_instances: bool
    :param autodispose_unregistered_instances: set autodispose_unregistered_instances (default: true)

    '''
    def __init__(self, autodispose_unregistered_instances = True):
        QosPolicy.__init__(self, QosPolicyId.WRITERDATALIFECYCLE_QOS_POLICY_ID)
        self._autodispose_unregistered_instances = autodispose_unregistered_instances

    @property
    def autodispose_unregistered_instances(self):
        return self._autodispose_unregistered_instances

class ReaderDataLifecycleQosPolicy(QosPolicy):
    '''
    Reader data lifecycle QoS Policy

    :type autopurge_nowriter_samples: DDSDuration
    :param autopurge_nowriter_samples: set autopurge_nowriter_samples (default: infinity)

    :type autopurge_disposed_samples_delay: DDSDuration
    :param autopurge_disposed_samples_delay: set autopurge_nowriter_samples (default: infinity)
    '''
    def __init__(self, autopurge_nowriter_samples = DDSDuration.infinity(), autopurge_disposed_samples_delay = DDSDuration.infinity()):
        QosPolicy.__init__(self, QosPolicyId.READERDATALIFECYCLE_QOS_POLICY_ID)
        self._autopurge_nowriter_samples = autopurge_nowriter_samples.value
        self._autopurge_disposed_samples_delay = autopurge_disposed_samples_delay.value

    @property
    def autopurge_nowriter_samples(self):
        return self._autopurge_nowriter_samples

    @property
    def autopurge_disposed_samples_delay(self):
        return self._autopurge_disposed_samples_delay

class DurabilityServiceQosPolicy(QosPolicy):
    '''
    Durability service QoS Policy

    :type service_cleanup_delay: DDSDuration
    :param service_cleanup_delay: set service_cleanup_delay (default: 0)

    :type history_kind: DDSHistoryKind
    :param history_kind: set history kind (default: KEEP_LAST)

    :type history_depth: long
    :param history_depth: set history_depth (default: 1)

    :type max_samples: long
    :param max_samples: set max_samples (default: unlimited)

    :type max_instances: long
    :param max_instances: set max_instances (default: unlimited)

    :type max_samples_per_instance: long
    :param max_samples_per_instance: set max_samples_per_instance (default: unlimited)

    '''
    def __init__(self, service_cleanup_delay = DDSDuration(0,0), history_kind = DDSHistoryKind.KEEP_LAST,
                 history_depth = 1, max_samples = DDS_LENGTH_UNLIMITED,
                 max_instances = DDS_LENGTH_UNLIMITED,
                 max_samples_per_instance = DDS_LENGTH_UNLIMITED):
        QosPolicy.__init__(self, QosPolicyId.DURABILITYSERVICE_QOS_POLICY_ID)
        self._service_cleanup_delay = service_cleanup_delay.value
        self._history_kind = history_kind
        self._history_depth = history_depth
        self._max_samples = max_samples
        self._max_instances = max_instances
        self._max_samples_per_instance = max_samples_per_instance

    @property
    def service_cleanup_delay(self):
        return self._service_cleanup_delay

    @property
    def history_kind(self):
        return self._history_kind

    @property
    def history_depth(self):
        return self._history_depth

    @property
    def max_samples(self):
        return self._max_samples

    @property
    def max_instances(self):
        return self._max_instances

    @property
    def max_samples_per_instance(self):
        return self._max_samples_per_instance

class UserdataQosPolicy(QosPolicy):
    '''
    Userdata QoS Policy

    :type value: string
    :param value: value

    '''
    def __init__(self, value = ''):
        QosPolicy.__init__(self, QosPolicyId.USERDATA_QOS_POLICY_ID)
        self._value = value

    @property
    def value(self):
        return self._value

class TopicdataQosPolicy(UserdataQosPolicy):
    '''
    Topic data QoS Policy

    :type value: string
    :param value: value

    '''
    def __init__(self, value = ''):
        QosPolicy.__init__(self, QosPolicyId.TOPICDATA_QOS_POLICY_ID)
        self._value = value

class GroupdataQosPolicy(UserdataQosPolicy):
    '''
    Group data QoS Policy

    :type value: string
    :param value: value

    '''
    def __init__(self, value = ''):
        QosPolicy.__init__(self, QosPolicyId.GROUPDATA_QOS_POLICY_ID)
        self._value = value

# The QoS wrapper.
cdef class Qos:
    '''
    QoS class

    :type policies: list
    :param policies: list of QoS policies
    '''
    cdef dds.dds_qos_t *_c_qos
    cdef object _policies

    # The init function takes a list of QosPolicies
    def __cinit__(self, policies=[]):
        self._c_qos = dds.dds_qos_create() #gil_ok
        self._policies = {}
        for p in policies:
            self._policies[p.id] = p
        self._apply_all()

    def __dealloc__(self):
        if self._c_qos is not NULL:
            dds.dds_qos_delete(self._c_qos) #gil_ok

    def set_policies(self, policies):
        '''set_policies(policies)
        Set policies
        :type policies: list
        :param policies: list of QoS policies
        '''
        for p in policies:
            self._policies[p.id] = p
            self._apply_policy(p)

    cdef _apply_all(self):
        for key, p in self._policies.items():
            self._apply_policy(p)

    cdef _apply_policy(self, policy):

        cdef char **_c_partitions = NULL;
        cdef char *_c_xxxdata_value = NULL
        cdef size_t _c_xxxdata_length = 0;

        if policy.id == QosPolicyId.PARTITION_QOS_POLICY_ID:
            partitions = policy.ps
            numpar = len(partitions)

            if numpar > 0:
                _c_partitions = <char **>PyMem_Malloc(numpar * sizeof(char *))
                if _c_partitions is NULL:
                    raise DDSException()
                for i in range(numpar):
                    s = partitions[i].encode()
                    _c_partitions[i] = s
            dds.dds_qset_partition(self._c_qos, numpar, <const char **>_c_partitions) #gil_ok
            PyMem_Free(_c_partitions)

        elif policy.id == QosPolicyId.USERDATA_QOS_POLICY_ID \
                or policy.id == QosPolicyId.TOPICDATA_QOS_POLICY_ID \
                or policy.id == QosPolicyId.GROUPDATA_QOS_POLICY_ID :
            bytes_value = None
            if isinstance(policy.value, str):
                bytes_value = policy.value.encode('ISO-8859-1')
            elif isinstance(policy.value, bytes):
                bytes_value = policy.value
            if bytes_value is not None:
                _c_xxxdata_value = <char *>bytes_value
                _c_xxxdata_length = len(bytes_value)
                if policy.id == QosPolicyId.USERDATA_QOS_POLICY_ID:
                    dds.dds_qset_userdata(self._c_qos, <const void *> _c_xxxdata_value, _c_xxxdata_length) #gil_ok
                elif policy.id == QosPolicyId.TOPICDATA_QOS_POLICY_ID:
                    dds.dds_qset_topicdata(self._c_qos, <const void *> _c_xxxdata_value, _c_xxxdata_length) #gil_ok
                elif policy.id == QosPolicyId.GROUPDATA_QOS_POLICY_ID:
                    dds.dds_qset_groupdata(self._c_qos, <const void *> _c_xxxdata_value, _c_xxxdata_length) #gil_ok

        elif policy.id == QosPolicyId.DURABILITY_QOS_POLICY_ID:
            dds.dds_qset_durability(self._c_qos, policy.kind.value) #gil_ok

        elif policy.id == QosPolicyId.HISTORY_QOS_POLICY_ID:
            dds.dds_qset_history(self._c_qos, policy.kind.value, policy.depth) #gil_ok

        elif policy.id == QosPolicyId.RESOURCELIMITS_QOS_POLICY_ID:
            dds.dds_qset_resource_limits(self._c_qos, policy.max_samples,
                                         policy.max_instances, policy.max_samples_per_instance) #gil_ok

        elif policy.id == QosPolicyId.PRESENTATION_QOS_POLICY_ID:
            dds.dds_qset_presentation(self._c_qos, policy.kind.value,
                                      int(policy.coherent_access), int(policy.ordered_access)) #gil_ok

        elif policy.id == QosPolicyId.LIFESPAN_QOS_POLICY_ID:
            dds.dds_qset_lifespan(self._c_qos, policy.lifespan) #gil_ok

        elif policy.id == QosPolicyId.DEADLINE_QOS_POLICY_ID:
            dds.dds_qset_deadline(self._c_qos, policy.deadline) #gil_ok

        elif policy.id == QosPolicyId.LATENCYBUDGET_QOS_POLICY_ID:
            dds.dds_qset_latency_budget(self._c_qos, policy.duration) #gil_ok

        elif policy.id == QosPolicyId.OWNERSHIP_QOS_POLICY_ID:
            dds.dds_qset_ownership(self._c_qos, policy.kind.value) #gil_ok

        elif policy.id == QosPolicyId.OWNERSHIPSTRENGTH_QOS_POLICY_ID:
            dds.dds_qset_ownership_strength(self._c_qos, policy.value) #gil_ok

        elif policy.id == QosPolicyId.LIVELINESS_QOS_POLICY_ID:
            dds.dds_qset_liveliness(self._c_qos, policy.kind.value, policy.lease_duration) #gil_ok

        elif policy.id == QosPolicyId.TIMEBASEDFILTER_QOS_POLICY_ID:
            dds.dds_qset_time_based_filter(self._c_qos, policy.minimum_separation) #gil_ok

        elif policy.id == QosPolicyId.RELIABILITY_QOS_POLICY_ID:
            dds.dds_qset_reliability(self._c_qos, policy.kind.value, policy.max_blocking_time) #gil_ok

        elif policy.id == QosPolicyId.TRANSPORTPRIORITY_QOS_POLICY_ID:
            dds.dds_qset_transport_priority(self._c_qos, policy.value) #gil_ok

        elif policy.id == QosPolicyId.DESTINATIONORDER_QOS_POLICY_ID:
            dds.dds_qset_destination_order(self._c_qos, policy.kind.value) #gil_ok

        elif policy.id == QosPolicyId.WRITERDATALIFECYCLE_QOS_POLICY_ID:
            dds.dds_qset_writer_data_lifecycle(self._c_qos,
                                               int(policy.autodispose_unregistered_instances)) #gil_ok

        elif policy.id == QosPolicyId.READERDATALIFECYCLE_QOS_POLICY_ID:
            dds.dds_qset_reader_data_lifecycle(self._c_qos,
                                               policy.autopurge_nowriter_samples,
                                               policy.autopurge_disposed_samples_delay) #gil_ok
        elif policy.id == QosPolicyId.DURABILITYSERVICE_QOS_POLICY_ID:
            dds.dds_qset_durability_service(self._c_qos, policy.service_cleanup_delay,
                                            policy.history_kind.value, policy.history_depth,
                                            policy.max_samples, policy.max_instances,
                                            policy.max_samples_per_instance) #gil_ok

    cdef dds.dds_qos_t * handle(self):
        return self._c_qos

# Functions to set the state mask
class DDSMaskUtil:
    '''
    Mask utility.
    Some commonly used mask combination
    See :class:`DDSStateKind` for list of available status
    '''
    @staticmethod
    def read_samples():
        '''
        [DDSStateKind.READ_SAMPLE_STATE, DDSStateKind.ALIVE_INSTANCE_STATE, DDSStateKind.ANY_VIEW_STATE]

        :rtype: list
        :return: list of :class:`.DDSStateKind` enumeration literals
        '''
        return [DDSStateKind.READ_SAMPLE_STATE, DDSStateKind.ALIVE_INSTANCE_STATE, DDSStateKind.ANY_VIEW_STATE]

    @staticmethod
    def new_samples():
        '''
        [DDSStateKind.NOT_READ_SAMPLE_STATE, DDSStateKind.ALIVE_INSTANCE_STATE, DDSStateKind.ANY_VIEW_STATE]

        :rtype: list
        :return: list of :class:`.DDSStateKind` enumeration literals
        '''
        return [DDSStateKind.NOT_READ_SAMPLE_STATE, DDSStateKind.ALIVE_INSTANCE_STATE, DDSStateKind.ANY_VIEW_STATE]

    @staticmethod
    def all_samples():
        '''
        [DDSStateKind.ANY_STATE]

        :rtype: list
        :return: list of :class:`.DDSStateKind` enumeration literals
        '''
        return [DDSStateKind.ANY_STATE]

    @staticmethod
    def new_instance_samples():
        '''
        [DDSStateKind.NOT_READ_SAMPLE_STATE, DDSStateKind.ALIVE_INSTANCE_STATE, DDSStateKind.NEW_VIEW_STATE]

        :rtype: list
        :return: list of :class:`.DDSStateKind` enumeration literals
        '''
        return [DDSStateKind.NOT_READ_SAMPLE_STATE, DDSStateKind.ALIVE_INSTANCE_STATE, DDSStateKind.NEW_VIEW_STATE]

    @staticmethod
    def not_alive_instance_samples():
        '''
        [DDSStateKind.ANY_SAMPLE_STATE, DDSStateKind.ANY_VIEW_STATE, DDSStateKind.NOT_ALIVE_NO_WRITERS_INSTANCE_STATE, DDSStateKind.NOT_ALIVE_DISPOSED_INSTANCE_STATE]

        :rtype: list
        :return: list of :class:`.DDSStateKind` enumeration literals
        '''
        return [DDSStateKind.ANY_SAMPLE_STATE, DDSStateKind.ANY_VIEW_STATE, DDSStateKind.NOT_ALIVE_NO_WRITERS_INSTANCE_STATE, DDSStateKind.NOT_ALIVE_DISPOSED_INSTANCE_STATE]

# Abstract listener
class Listener(object):
    '''
    Abstract Listener class.
    '''

    def on_inconsistent_topic(self, topic, status):
        '''
        A participant registered a topic inconsistent with a locally registered one.

        This notification applies to: Topic and DomainParticipant

        Note: this event is unlikely in OpenSplice because OpenSplice does
        not allow inconsistent topics to register. It may, however, occur
        in heterogeneous domains with participants using other DDS stacks.

        :param topic: a handle to the locally registered topic
        :type topic: dds.Topic
        :param status: a namedtuple with fields: total_count, total_count_change
        :param status: dds.InconsistentTopicStatus
        '''
        pass

    def on_offered_deadline_missed(self, writer, status):
        '''
        A locally created data writer has failed to adhere it's DeadlinePolicy.

        This notification applies to: DataWriter, Publisher and DomainParticipant

        :param writer: a handle to the writer that missed is write deadline
        :type writer: dds.DataWriter
        :param status: a namedtuple with fields: total_count, total_count_change, last_instance_handle
        :type status: dds.OfferedDeadlineMissedStatus
        '''
        pass

    def on_offered_incompatible_qos(self, writer, status):
        '''
        A data writer could not be matched with a data reader because of Qos incompatibilities

        This notification applies to: DataWriter, Publisher and DomainParticipant

        :param writer: a handle to the writer
        :type writer: dds.DataWriter
        :param status: a namedtuple with fields: total_count, total_count_change, last_policy_id
        :type status: dds.OfferedIncompatibleQosStatus
        '''
        pass

    def on_liveliness_lost(self, writer, status):
        '''
        A data writer has not met its committed LivelinessQosPolicy

        This notification applies to: DataWriter, Publisher and DomainParticipant

        :param writer: a handle to the writer
        :type writer: dds.DataWriter
        :param status: a namedtuple with fields: total_count, total_count_change
        :type status: dds.LivelinessLostStatus
        '''
        pass

    def on_publication_matched(self, writer, status):
        '''
        The number of data readers matched to writer has changed.

        This notification applies to: DataWriter, Publisher and DomainParticipant

        :param writer: a handle to the writer
        :type writer: dds.DataWriter
        :param status: a namedtuple with fields: total_count, total_count_change, current_count, current_count_change, last_subscription_handle
        :type status: dds.PublicationMatchedStatus
        '''
        pass

    def on_requested_deadline_missed(self, reader, status):
        '''
        A reader's requested DeadlinePolicy was not met by it's matched writers.

        This notification applies to: DataReader, Subscriber and DomainParticipant

        :param reader: a handle to the reader
        :type reader: dds.DataReader
        :param status: a namedtuple with fields: total_count, total_count_change, last_instance_handle
        :type status: dds.RequestedDeadlineMissedStatus
        '''
        pass

    def on_requested_incompatible_qos(self, reader, status):
        '''
        A data reader could not be matched with a data writer because of Qos incompatibilities

        This notification applies to: DataReader, Subscriber and DomainParticipant

        :param reader: a handle to the reader
        :type reader: dds.DataReader
        :param status: a namedtuple with fields: total_count, total_count_change, last_policy_id
        :type status: dds.RequestedIncompatibleQosStatus
        '''
        pass

    def on_sample_rejected(self, reader, status):
        '''
        The data reader did not receive samples because of the ResourceLimitsQosPolicy

        This notificiation applies to: DataReader, Subscriber and DomainParticipant

        :param reader: a handle to the reader
        :type reader: dds.DataReader
        :param status: a namedtuple with fields: total_count,total_count_change,last_reason,last_instance_handle
        :type status: dds.SampleRejectedStatus
        '''
        pass

    def on_liveliness_changed(self, reader, status):
        '''on_liveliness_changed(reader, status)
        A data reader has experienced a liveliness change in a match writer

        This notification applies to: DataReader, Subscriber and DomainParticipant

        :param reader: a handle to the reader
        :type reader: dds.DataReader
        :param status: a namedtuple with fields: alive_count, not_alive_count, alive_count_change, not_alive_count_change, last_publication_handle
        :type status: dds.LivelinessChangedStatus
        '''
        pass

    def on_data_available(self, reader):
        '''
        Data is available on the passed data reader.

        This notificiation applies: DataReader, Subscriber or DomainParticipant

        :param reader: a handle to the reader
        :type reader: dds.DataReader
        '''
        pass

    def on_subscription_matched(self, reader, status):
        '''
        The number of data writers matched to a reader has changed.

        This notification applies to: DataReader, Subscriber and DomainParticipant

        :param reader: a handle to the reader
        :type reader: dds.DataReader
        :param status: a namedtuple with fields: total_count, total_count_change, current_count, current_count_change, last_subscription_handle
        :type status: dds.SubscriptionMatchedStatus
        '''
        pass

    def on_sample_lost(self, reader, status):
        '''
        A sample for the data reader has been lost

        This notification applies to DataReader, Subscriber and DomainParticipant

        :param reader: a handle to the reader
        :type reader: dds.DataReader
        :param status: a namedtuple with fields: total_count, total_count_change
        :type status: dds.SampleLostStatus
        '''
        pass

    def on_data_on_readers(self, subscriber):
        '''
        One or more data readers belonging to the subscriber have data available.

        This notification applies to: Subscriber and Domain Participant

        Note 1: The underlying C99 API supports neither Subscriber.get_datareaders()
        nor Subscriber.notifiy_datareaders(), making this listener only marginally useful.
        Note 2: Per the DDS specification, registering this handler disables on_data_available() on all
        contained data readers.
        '''
        pass


# The abstract entity class contains the c handle to the C99 entity
# The handle and the entity are registered with a global dictionary
# to allow a listener callback to find the associated entity
cdef class Entity(object):
    '''
    Abstract base entity class

    '''
    cdef dds.dds_entity_t _c_handle
    cdef object _parent
    cdef object _listener

    def __cinit__(self):
        self._c_handle = NULL
        self._listener = None

    def __dealloc__(self):
        if self._c_handle is not NULL:
            with nogil:
                dds.dds_entity_delete(<void*>self._c_handle)
        _entity_register_remove_children(self)
        _entity_register_remove(self)
        self._c_handle = NULL
        self._listener = None

    def close(self):
        '''
        Delete this DDS entity. Recursively deletes contained
        entities before deleting this entity.
        '''
        self._listener = None
        if self._c_handle is not NULL:
            with nogil:
                dds.dds_entity_delete(<void*>self._c_handle)
        # dds_entity_delete recursively frees the C entities,
        # so we must do the same cleanup for the Python entities,
        # so as to avoid triggering a double delete.
        _entity_register_remove_children(self)
        _entity_register_remove(self)
        self._c_handle = NULL

    def _check_handle(self):
        if self._c_handle is NULL:
            raise DDSException('Entity is already closed')

    cdef set_handle(self, dds.dds_entity_t h):
        self._c_handle = h

    cdef dds.dds_entity_t get_handle(self):
        return self._c_handle

    @property
    def parent(self):
        ''' Parent entity
        '''
        return self._parent

    @parent.setter
    def parent(self, Entity e):
        self._parent = e

    cdef dds.dds_entity_t handle(self):
        return self._c_handle

    @property
    def listener(self):
        ''' Listener
        '''
        return self._listener

    @listener.setter
    def listener(self, listener):
        self._listener = listener

    def get_statuscondition(self):
        return StatusCondition(self)


# Global variable to map the C99 entity handles on the corresponding Entity class
cdef object _entity_register = {}

cdef _entity_register_add(Entity entity):
    key = <intptr_t>entity.handle()
    #print('entity_register_add: key={}'.format(key))
    _entity_register[key] = entity

cdef _entity_register_remove(Entity entity):
    key = <intptr_t>entity.handle()
    if key != 0 and key in _entity_register:
        del _entity_register[key]

cdef _entity_register_remove_children(Entity entity):
    if <intptr_t>entity.handle() != 0:
        child_entities = []
        for key in _entity_register:
            e = <Entity>_entity_register[key]
            p = e.parent
            while p:
                if p == entity:
                    break
                else:
                    p = p.parent
            if p:
                child_entities.append(_entity_register[<intptr_t>e.handle()])
        for child in child_entities:
            echild = <Entity>child
            del _entity_register[<intptr_t>echild.handle()]
            echild.set_handle(NULL)

cdef Entity _entity_register_find(dds_entity_t e):
    key = <intptr_t>e
    #print('entity_register_find: key={}'.format(key))
    if key != 0 and key in _entity_register:
        return _entity_register[key]
    return None

# unpacking dds API char * values to Python Strings
cdef str _to_pystr_and_free(const char *c_str):
    try:
        return c_str.decode('ISO-8859-1','strict')
    finally:
        dds_free(<void*>c_str) #gil_ok

# map listener methods to status defines
_method_to_status = {
    "on_inconsistent_topic": DDS_INCONSISTENT_TOPIC_STATUS,
    "on_offered_deadline_missed": DDS_OFFERED_DEADLINE_MISSED_STATUS,
    "on_requested_deadline_missed": DDS_REQUESTED_DEADLINE_MISSED_STATUS,
    "on_offered_incompatible_qos": DDS_OFFERED_INCOMPATIBLE_QOS_STATUS,
    "on_requested_incompatible_qos": DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS,
    "on_sample_lost": DDS_SAMPLE_LOST_STATUS,
    "on_sample_rejected": DDS_SAMPLE_REJECTED_STATUS,
    "on_data_on_readers": DDS_DATA_ON_READERS_STATUS,
    "on_data_available": DDS_DATA_AVAILABLE_STATUS,
    "on_liveliness_lost": DDS_LIVELINESS_LOST_STATUS,
    "on_liveliness_changed": DDS_LIVELINESS_CHANGED_STATUS,
    "on_publication_matched": DDS_PUBLICATION_MATCHED_STATUS,
    "on_subscription_matched": DDS_SUBSCRIPTION_MATCHED_STATUS,
    }
# Communication Status structures
class InconsistentTopicStatus(namedtuple('InconsistentTopicStatus',[
    'total_count',
    'total_count_change'])):
    ''' InconsistentTopicStatus

    total_count -> integer
        Total cumulative count of the Topics discovered whose name matches
        the Topic to which this status is attached and whose type is
        inconsistent with the Topic.

    total_count_change -> integer
        The incremental number of inconsistent topics discovered since the
        last time the listener was called or the status was read.
    '''

class OfferedDeadlineMissedStatus(namedtuple('OfferedDeadlineMissedStatus',[
    'total_count',
    'total_count_change',
    'last_instance_handle'])):
    ''' OfferedDeadlineMissedStatus

    total_count -> integer
        Total cumulative number of offered deadline periods elapsed during
        which a DataWriter failed to provide data. Missed deadlines
        accumulate; that is, each deadline period the total_count will be
        incremented by one.

    total_count_change -> integer
        The change in total_count since the last time the listener was called or
        the status was read.

    last_instance_handle -> integer
        Handle to the last instance in the DataWriter for which an offered
        deadline was missed.
    '''

class OfferedIncompatibleQosStatus(namedtuple('OfferedIncompatibleQosStatus',[
    'total_count',
    'total_count_change',
    'last_policy_id'])):
    ''' OfferedIncompatibleQosStatus

    total_count -> integer
        Total cumulative number of times the concerned DataWriter
        discovered a DataReader for the same Topic with a requested QoS that
        is incompatible with that offered by the DataWriter.

    total_count_change -> integer
        The change in total_count since the last time the listener was called or
        the status was read.

    last_policy_id -> dds.QosPolicyId
        The QosPolicyId of one of the policies that was found to be
        incompatible the last time an incompatibility was detected.
    '''

class LivelinessLostStatus(namedtuple('LivelinessLostStatus',[
    'total_count',
    'total_count_change'])):
    ''' LivelinessLostStatus

    total_count -> integer
        Total cumulative number of times that a previously-alive DataWriter
        became not alive due to a failure to actively signal its liveliness within
        its offered liveliness period. This count does not change when an
        already not alive DataWriter simply remains not alive for another
        liveliness period.

    total_count_change -> integer
        The change in total_count since the last time the listener was called or
        the status was read.
    '''

class PublicationMatchedStatus(namedtuple('PublicationMatchedStatus',[
    'total_count',
    'total_count_change',
    'current_count',
    'current_count_change',
    'last_subscription_handle'])):
    ''' PublicationMatchedStatus
    total_count -> integer
        Total cumulative count the concerned DataWriter discovered a
        "match" with a DataReader. That is, it found a DataReader for the
        same Topic with a requested QoS that is compatible with that offered
        by the DataWriter.

    total_count_change -> integer
        The change in total_count since the last time the listener was called or
        the status was read.

    current_count -> integer
        The number of DataReaders currently matched to the concerned
        DataWriter.

    current_count_change -> integer
        The change in current_count since the last time the listener was called
        or the status was read.

    last_subscription_handle -> integer
        Handle to the last DataReader that matched the DataWriter causing the
        status to change.
    '''

class RequestedDeadlineMissedStatus(namedtuple('RequestedDeadlineMissedStatus',[
    'total_count',
    'total_count_change',
    'last_instance_handle'])):
    ''' RequestedDeadlineMissedStatus

    total_count -> integer
        Total cumulative number of missed deadlines detected for any instance
        read by the DataReader. Missed deadlines accumulate; that is, each
        deadline period the total_count will be incremented by one for each
        instance for which data was not received.

    total_count_change -> integer
        The incremental number of deadlines detected since the last time the
        listener was called or the status was read.

    last_instance_handle -> integer
        Handle to the last instance in the DataReader for which a deadline was
        detected.
    '''

class RequestedIncompatibleQosStatus(namedtuple('RequestedIncompatibleQosStatus',[
    'total_count',
    'total_count_change',
    'last_policy_id'])):
    ''' RequestedIncompatibleQosStatus
    total_count -> integer
        Total cumulative number of times the concerned DataReader
        discovered a DataWriter for the same Topic with an offered QoS that
        was incompatible with that requested by the DataReader.

    total_count_change -> integer
        The change in total_count since the last time the listener was called or
        the status was read.

    last_policy_id -> dds.QosPolicyId
        The QosPolicyId of one of the policies that was found to be
        incompatible the last time an incompatibility was detected.
    '''

class SampleRejectedStatus(namedtuple('SampleRejectedStatus',[
    'total_count',
    'total_count_change',
    'last_reason',
    'last_instance_handle'])):
    ''' SampleRejectedStatus

    total_count -> integer
        Total cumulative count of samples rejected by the DataReader.

    total_count_change -> integer
        The incremental number of samples rejected since the last time the
        listener was called or the status was read.

    last_reason -> dds.DDSSampleRejectedStatusKind
        Reason for rejecting the last sample rejected. If no samples have been
        rejected, the reason is the special value NOT_REJECTED.

    last_instance_handle -> integer
        Handle to the instance being updated by the last sample that was
        rejected.
    '''

class LivelinessChangedStatus(namedtuple('LivelinessChangedStatus',[
    'alive_count',
    'not_alive_count',
    'alive_count_change',
    'not_alive_count_change',
    'last_publication_handle'])):
    ''' LivelinessChangedStatus

    alive_count -> integer
        The total number of currently active DataWriters that write the Topic
        read by the DataReader. This count increases when a newly matched
        DataWriter asserts its liveliness for the first time or when a DataWriter
        previously considered to be not alive reasserts its liveliness. The count
        decreases when a DataWriter considered alive fails to assert its
        liveliness and becomes not alive, whether because it was deleted
        normally or for some other reason.

    not_alive_count -> integer
        The total count of currently DataWriters that write the Topic read by
        the DataReader that are no longer asserting their liveliness. This count
        increases when a DataWriter considered alive fails to assert its
        liveliness and becomes not alive for some reason other than the normal
        deletion of that DataWriter. It decreases when a previously not alive
        DataWriter either reasserts its liveliness or is deleted normally.

    alive_count_change -> integer
        The change in the alive_count since the last time the listener was
        called or the status was read.

    not_alive_count_change -> integer
        The change in the not_alive_count since the last time the listener was
        called or the status was read.

    last_publication_handle -> integer
        Handle to the last DataWriter whose change in liveliness caused this
        status to change.
    '''

class SubscriptionMatchedStatus(namedtuple('SubscriptionMatchedStatus',[
    'total_count',
    'total_count_change',
    'current_count',
    'current_count_change',
    'last_publication_handle'])):
    ''' SubscriptionMatchedStatus

    total_count -> integer
        Total cumulative count the concerned DataReader discovered a
        "match" with a DataWriter. That is, it found a DataWriter for the same
        Topic with a requested QoS that is compatible with that offered by the
        DataReader.

    total_count_change -> integer
        The change in total_count since the last time the listener was called or
        the status was read.

    current_count -> integer
        The number of DataWriters currently matched to the concerned
        DataReader.

    current_count_change -> integer
        The change in current_count since the last time the listener was called
        or the status was read.

    last_publication_handle -> integer
        Handle to the last DataWriter that matched the DataReader causing the
        status to change.
    '''

class SampleLostStatus(namedtuple('SampleLostStatus',[
    'total_count',
    'total_count_change'])):
    ''' SampleLostStatus

    total_count -> integer
        Total cumulative count of all samples lost across of instances of data
        published under the Topic.

    total_count_change -> integer
        The incremental number of samples lost since the last time the listener
        was called or the status was read.
    '''

cdef uint32_t _status_mask_for_listener(object listener):
    cdef uint32_t mask = 0
#     statuses = []
    for name, _ in inspect.getmembers(type(listener), predicate=inspect.isfunction):
        if name in _method_to_status:
#             statuses.append(name)
            mask |= _method_to_status[name]
#     print('_status_mask_for_listener: mask = {:x} for status: {}'.format(mask, ','.join(statuses)))
    return mask

# Implementation of the Listener callback routines. Currently only the on_data_available is implemented
cdef void on_inconsistent_topic (dds_entity_t topic, dds.dds_inconsistent_topic_status_t * status) with gil:
    cdef object entity = _entity_register_find(topic)
    cdef uint32_t total_count = status.total_count
    cdef int32_t total_count_change = status.total_count_change

    if entity:
        listener = entity.listener
        if listener:
            listener.on_inconsistent_topic(entity,InconsistentTopicStatus(total_count, total_count_change))

cdef void on_offered_deadline_missed (dds_entity_t writer, dds_offered_deadline_missed_status_t * status) with gil:
    cdef object entity = _entity_register_find(writer)
    cdef uint32_t total_count = status.total_count
    cdef int32_t total_count_change = status.total_count_change
    cdef dds_instance_handle_t last_instance_handle = status.last_instance_handle

    if entity:
        listener = entity.listener
        if listener:
            listener.on_offered_deadline_missed(entity,OfferedDeadlineMissedStatus(total_count, total_count_change, last_instance_handle))

cdef void on_offered_incompatible_qos (dds_entity_t writer, dds_offered_incompatible_qos_status_t * status) with gil:
    cdef object entity = _entity_register_find(writer)
    cdef uint32_t total_count = status.total_count
    cdef int32_t total_count_change = status.total_count_change
    cdef uint32_t last_policy_id = status.last_policy_id

    if entity:
        listener = entity.listener
        if listener:
            listener.on_offered_incompatible_qos(entity,OfferedIncompatibleQosStatus(total_count, total_count_change, QosPolicyId(last_policy_id)))

cdef void on_liveliness_lost (dds_entity_t writer, dds_liveliness_lost_status_t * status) with gil:
    cdef object entity = _entity_register_find(writer)
    cdef uint32_t total_count = status.total_count
    cdef int32_t total_count_change = status.total_count_change

    if entity:
        listener = entity.listener
        if listener:
            listener.on_liveliness_lost(entity,LivelinessLostStatus(total_count, total_count_change))

cdef void on_publication_matched (dds_entity_t writer, dds_publication_matched_status_t * status) with gil:
    cdef object entity = _entity_register_find(writer)
    cdef uint32_t total_count = status.total_count
    cdef int32_t total_count_change = status.total_count_change
    cdef uint32_t current_count = status.current_count
    cdef int32_t current_count_change = status.current_count_change
    cdef dds_instance_handle_t last_subscription_handle = status.last_subscription_handle

    if entity:
        listener = entity.listener
        if listener:
            listener.on_publication_matched(entity,PublicationMatchedStatus(total_count, total_count_change, current_count, current_count_change, last_subscription_handle))

cdef void on_requested_deadline_missed (dds_entity_t reader, dds_requested_deadline_missed_status_t * status) with gil:
    cdef object entity = _entity_register_find(reader)
    cdef uint32_t total_count = status.total_count
    cdef int32_t total_count_change = status.total_count_change
    cdef dds_instance_handle_t last_instance_handle = status.last_instance_handle

    if entity:
        listener = entity.listener
        if listener:
            listener.on_requested_deadline_missed(entity,RequestedDeadlineMissedStatus(total_count, total_count_change, last_instance_handle))

cdef void on_requested_incompatible_qos (dds_entity_t reader, dds_requested_incompatible_qos_status_t * status) with gil:
    cdef object entity = _entity_register_find(reader)
    cdef uint32_t total_count = status.total_count
    cdef int32_t total_count_change = status.total_count_change
    cdef uint32_t last_policy_id = status.last_policy_id

    if entity:
        listener = entity.listener
        if listener:
            listener.on_requested_incompatible_qos(entity,RequestedIncompatibleQosStatus(total_count, total_count_change, QosPolicyId(last_policy_id)))

cdef void on_sample_rejected (dds_entity_t reader, dds_sample_rejected_status_t * status) with gil:
    cdef object entity = _entity_register_find(reader)
    cdef uint32_t total_count = status.total_count
    cdef int32_t total_count_change = status.total_count_change
    cdef dds_sample_rejected_status_kind last_reason = status.last_reason
    cdef dds_instance_handle_t last_instance_handle = status.last_instance_handle

    if entity:
        listener = entity.listener
        if listener:
            listener.on_sample_rejected(entity,SampleRejectedStatus(total_count, total_count_change,
                                                         DDSSampleRejectedStatusKind(last_reason), last_instance_handle))

cdef void on_liveliness_changed (dds_entity_t reader, dds_liveliness_changed_status_t * status) with gil:
    cdef object entity = _entity_register_find(reader)
    cdef uint32_t alive_count = status.alive_count
    cdef uint32_t not_alive_count = status.not_alive_count
    cdef int32_t alive_count_change = status.alive_count_change
    cdef int32_t not_alive_count_change = status.not_alive_count_change
    cdef dds_instance_handle_t last_publication_handle = status.last_publication_handle

    if entity:
        listener = entity.listener
        if listener:
            listener.on_liveliness_changed(entity,LivelinessChangedStatus(alive_count,not_alive_count,alive_count_change,not_alive_count_change,last_publication_handle))

cdef void on_data_available (dds_entity_t reader) with gil:
    cdef object entity = _entity_register_find(reader)

    if entity:
        listener = entity.listener
        if listener:
            listener.on_data_available(entity)

cdef void on_subscription_matched (dds_entity_t reader, dds_subscription_matched_status_t * status) with gil:
    cdef object entity = _entity_register_find(reader)
    cdef uint32_t total_count = status.total_count
    cdef int32_t total_count_change = status.total_count_change
    cdef uint32_t current_count = status.current_count
    cdef int32_t current_count_change = status.current_count_change
    cdef dds_instance_handle_t last_publication_handle = status.last_publication_handle

    if entity:
        listener = entity.listener
        if listener:
            listener.on_subscription_matched(entity,SubscriptionMatchedStatus(total_count, total_count_change, current_count, current_count_change, last_publication_handle))

cdef void on_sample_lost (dds_entity_t reader, dds_sample_lost_status_t * status) with gil:
    cdef object entity = _entity_register_find(reader)
    cdef uint32_t total_count = status.total_count
    cdef int32_t total_count_change = status.total_count_change

    if entity:
        listener = entity.listener
        if listener:
            listener.on_sample_lost(entity,SampleLostStatus(total_count, total_count_change))

cdef void on_data_on_readers (dds_entity_t subscriber) with gil:
    cdef object entity = _entity_register_find(subscriber)

    if entity:
        listener = entity.listener
        if listener:
            listener.on_data_on_readers(entity)


# For each entity listener a global variable is used which is used
# to wrap the C listener and call the corresponding Python listener
cdef dds.dds_topiclistener_t topic_listener
topic_listener.on_inconsistent_topic = on_inconsistent_topic

cdef dds.dds_writerlistener_t writer_listener
writer_listener.on_offered_deadline_missed = on_offered_deadline_missed
writer_listener.on_offered_incompatible_qos = on_offered_incompatible_qos
writer_listener.on_liveliness_lost = on_liveliness_lost
writer_listener.on_publication_matched = on_publication_matched

cdef dds.dds_readerlistener_t reader_listener
reader_listener.on_requested_deadline_missed = on_requested_deadline_missed
reader_listener.on_requested_incompatible_qos = on_requested_incompatible_qos
reader_listener.on_sample_rejected = on_sample_rejected
reader_listener.on_liveliness_changed = on_liveliness_changed
reader_listener.on_data_available = on_data_available
reader_listener.on_subscription_matched = on_subscription_matched
reader_listener.on_sample_lost = on_sample_lost

cdef dds.dds_publisherlistener_t publisher_listener
publisher_listener.writerlistener = writer_listener

cdef dds.dds_subscriberlistener_t subscriber_listener
subscriber_listener.readerlistener = reader_listener
subscriber_listener.on_data_readers = on_data_on_readers

cdef dds.dds_participantlistener_t participant_listener
participant_listener.topiclistener = topic_listener
participant_listener.publisherlistener = publisher_listener
participant_listener.subscriberlistener = subscriber_listener

cdef _set_status_for_listener(dds_entity_t c_entity, object listener):
    cdef uint32_t listener_mask = _status_mask_for_listener(listener)
    cdef int r
    # this must be done as an indivisible step with listener registration
    r = dds.dds_status_set_enabled(c_entity, listener_mask) #need_gil
    if r < 0:
        raise DDSException('Error setting entity status for listener', r)


# The DomainParticipan wrapper
# Note that the proper Exceptions are not yet defined and for
# all possible exceptions the standard DDSException is used.
cdef class DomainParticipant(Entity):
    '''
    Domain participant class

    :type qos: Qos
    :param qos: Participant QoS. Default: None

    :type listener: Listener
    :param listener: Participant listener Default: None
    '''

    cdef object _publishers
    cdef object _subscribers
    cdef object _writers
    cdef object _readers

    def __cinit__(self, int did = DDS_DOMAIN_DEFAULT, dds.Qos qos = None, listener = None):

        self._publishers = []
        self._subscribers = []
        self._writers = []
        self._readers = []
        self._listener = listener

        cdef dds.dds_qos_t *_c_qos = NULL

        if qos and not isinstance(qos, Qos):
            raise DDSException('Bad parameter')

        if listener and not isinstance(listener, Listener):
            raise DDSException('Bad parameter')

        if qos:
            _c_qos = qos.handle()

        cdef dds_participantlistener_t *_c_listener = NULL
        if listener:
            _c_listener = &participant_listener

        # enabling a listener requires three steps, done as a indivisible action:
        #   1) dds_participant_create
        #   2) _entity_register_add
        #   3) _set_status_for_listener
        # The easiest way is to NOT release the GIL during these.
        r = dds_participant_create(&self._c_handle, did, _c_qos, <dds_participantlistener_t *>_c_listener) # need_gil
        if r < 0:
            raise DDSException('Failed to create participant', r)

        _entity_register_add(self)
        if listener:
            _set_status_for_listener(self._c_handle, listener)
        # let any triggered listeners run by yielding to other threads
        time.sleep(0)

    def create_publisher(self, Qos qos = None, listener = None):
        '''create_publisher(Qos qos = None, listener = None)
        Create a publisher

        :type qos: QoS
        :param qos: QoS

        :type listener: Listener
        :param listener: Listener

        :rtype: Publisher
        '''
        pub = Publisher(self, qos, listener)
        self._publishers.append(pub)
        return pub

    def create_datawriter(self, Topic topic, Qos qos = None, listener = None):
        '''create_datawriter(Topic topic, Qos qos = None, listener = None)
        Create a publisher

        :type topic: Topic
        :param topic: Topic

        :type qos: QoS
        :param qos: QoS

        :type listener: Listener
        :param listener: Listener

        :rtype: DataWriter
        '''
        writer = DataWriter(self, topic, qos, listener)
        self._writers.append(writer)
        return writer

    def create_subscriber(self, Qos qos = None, listener = None):
        '''create_subscriber(Qos qos = None, listener = None)
        Create a subscriber

        :type qos: QoS
        :param qos: QoS

        :type listener: Listener
        :param listener: Listener

        :rtype: Subscriber
        '''
        sub = Subscriber(self, qos, listener)
        self._subscribers.append(sub)
        return sub

    def create_datareader(self, Topic topic, Qos qos = None, listener = None):
        '''create_datareader(Topic topic, Qos qos = None, listener = None)
        Create a publisher

        :type topic: Topic
        :param topic: Topic

        :type qos: QoS
        :param qos: QoS

        :type listener: Listener
        :param listener: Listener

        :rtype: DataReader
        '''
        reader = DataReader(self, topic, qos, listener)
        self._readers.append(reader)
        return reader

    def find_topic(self, str name):
        '''find_topic(str name, listener = None)
        Find an already registered topic, and make it available locally

        :type name: str
        :param name: the topic name

        :rtype: Topic
        '''
        cdef dds.dds_entity_t c_topic
        name_as_bytes = name.encode('ISO-8859-1')
        cdef char *name_as_char = name_as_bytes
        cdef dds_entity_t c_entity = self.handle()
        with nogil:
            c_topic = dds.dds_topic_find(c_entity, name_as_char)
        if c_topic != <dds_entity_t>0:
            return _FoundTopic_Init(self, c_topic)
        else:
            return None


# Abstract class which represents the Topic type
# It is used to generate a dds_topic_descriptor from the
# provided typename, keys and XML type specification.
# The size attribute is need to allocate memory when
# the topic data is deserialized.
# For the serialization and deserialization the struct
# module is used
cdef class TypeSupport:
    '''
    It is used to generate a dds_topic_descriptor from the
    provided typename, keys and XML type specification.
    The size attribute is need to allocate memory when
    the topic data is deserialized.
    For the serialization and deserialization the struct
    module is used
    '''

    def __init__(self, name, keys, spec, size):
        self._name = name
        self._keys = keys
        self._spec = spec
        self._size = size

    @property
    def handle(self):
        '''C handle
        '''
        return self._handle

    def _serialize(self, o):
        '''_serialize(o)
        Serialize data
        '''
        raise NotImplementedError('Subclass must override this implementation')

    @property
    def size(self):
        '''Data size
        '''
        return self._size

    def _deserialize(self, b):
        '''_deserialize(b)
        Deserialize data
        '''
        raise NotImplementedError('Subclass must override this implementation')

# Function to create a C99 topic descriptor from a TypeSupport
cdef const dds.dds_topic_descriptor_t * get_descriptor(ts):
    cdef dds.dds_topic_descriptor_t *c_descriptor
    cdef char *c_name
    cdef char *c_keys
    cdef char *c_spec
    if ts._name == 'DDS::ParticipantBuiltinTopicData':
        return &DDS_ParticipantBuiltinTopicData_desc
    elif ts._name == 'DDS::TopicBuiltinTopicData':
        return &DDS_TopicBuiltinTopicData_desc
    elif ts._name == 'DDS::PublicationBuiltinTopicData':
        return &DDS_PublicationBuiltinTopicData_desc
    elif ts._name == 'DDS::SubscriptionBuiltinTopicData':
        return &DDS_SubscriptionBuiltinTopicData_desc
    elif ts._name == 'DDS::CMParticipantBuiltinTopicData':
        return &DDS_CMParticipantBuiltinTopicData_desc
    elif ts._name == 'DDS::CMPublisherBuiltinTopicData':
        return &DDS_CMPublisherBuiltinTopicData_desc
    elif ts._name == 'DDS::CMSubscriberBuiltinTopicData':
        return &DDS_CMSubscriberBuiltinTopicData_desc
    elif ts._name == 'DDS::CMDataWriterBuiltinTopicData':
        return &DDS_CMDataWriterBuiltinTopicData_desc
    elif ts._name == 'DDS::CMDataReaderBuiltinTopicData':
        return &DDS_CMDataReaderBuiltinTopicData_desc
    elif ts._name == 'DDS::TypeBuiltinTopicData':
        return &DDS_TypeBuiltinTopicData_desc
    else:
        name_as_bytes = ts._name.encode('ISO-8859-1')
        keys_as_bytes = ts._keys.encode('ISO-8859-1')
        spec_as_bytes = ts._spec.encode('ISO_8859-1')
        c_name = name_as_bytes
        c_keys = keys_as_bytes
        c_spec = spec_as_bytes
        with nogil:
            c_descriptor = dds_topic_descriptor_create(c_name, c_keys, c_spec)
        return c_descriptor

# The Topic wrapper
cdef class Topic(Entity):
    '''
    Topic class

    :type dp: DomainParticipant
    :param dp: Domain participant

    :type name: string
    :param name: Topic name

    :type qos: Qos
    :param qos: Topic QoS (default: None)

    :type listener: Listener
    :param listener: Topic listener (default: None)

    '''
    cdef object _typesupport

    def __init__(self, DomainParticipant dp, name, ts, Qos qos = None, listener = None):
        dp._check_handle()
        cdef dds_qos_t *_c_qos = NULL
        if qos:
            _c_qos = qos.handle()

        if not isinstance(dp, DomainParticipant):
            raise DDSException('Bad parameter')

        if not isinstance(ts, TypeSupport):
            raise DDSException('Bad parameter')

        if qos and not isinstance(qos, Qos):
            raise DDSException('Bad parameter')

        if listener and not isinstance(listener, Listener):
            raise DDSException('Bad parameter')


        self._typesupport = ts
        self.parent = dp
        self._listener = listener

        cdef dds_topiclistener_t *_c_listener = NULL
        if listener:
            _c_listener = &topic_listener

        cdef const dds_topic_descriptor_t *descriptor = get_descriptor(ts)
        name_as_bytes = name.encode('ISO-8859-1')
        cdef char *c_name = name_as_bytes
        # enabling a listener requires three steps, done as a indivisible action:
        #   1) dds_topic_create
        #   2) _entity_register_add
        #   3) _set_status_for_listener
        # The easiest way is to NOT release the GIL during these.
        r = dds.dds_topic_create(dp._c_handle, &self._c_handle, descriptor, c_name, _c_qos, <dds_topiclistener_t *>_c_listener) # need_gil
        if name not in _builtin_topics_descriptors.keys():
            dds.dds_topic_descriptor_delete(<dds_topic_descriptor_t *>descriptor) #gil_ok
        if r < 0:
            raise DDSException('Failed to create topic.', r)

        _entity_register_add(self)
        if listener:
            _set_status_for_listener(self._c_handle, listener)
        # let any triggered listeners run by yielding to other threads
        time.sleep(0)

    @property
    def name(self):
        '''
        The topic name, as registered in the DDS domain

        :rtype: str
        '''
        self._check_handle()
        cdef char *cstr = dds.dds_topic_get_name(self._c_handle) #gil_ok
        return _to_pystr_and_free(cstr)

    @property
    def type_name(self):
        '''
        The fully qualified name of the topic data type (in IDL format)

        :rtype: str
        '''
        self._check_handle()
        cdef char *cstr = dds.dds_topic_get_type_name(self._c_handle) #gil_ok
        type_name = _to_pystr_and_free(cstr)
        if type_name in _builtin_topics_type_name_map.keys():
            type_name = _builtin_topics_type_name_map[type_name]
        return type_name

    @property
    def metadescriptor(self):
        '''
        The XML meta-descriptor of the topic

        :rtype: str
        '''
        self._check_handle()
        cdef char *cstr
        if self.name in _builtin_topics_descriptors.keys():
            return _builtin_topics_descriptors[self.name]
        else:
            cstr = dds.dds_topic_get_metadescriptor(self._c_handle) #gil_ok
            return _to_pystr_and_free(cstr)

    @property
    def keylist(self):
        '''
        The possibly empty list of topic key fields, separated by commas.

        :rtype: str
        '''
        self._check_handle()
        cdef char *cstr = dds.dds_topic_get_keylist(self._c_handle) #gil_ok
        return _to_pystr_and_free(cstr)

    @property
    def qos(self):
        '''
        Return the QoS policy for the entity

        :rtype: dds.Qos
        '''
        qos = Qos();
        cdef dds_entity_t c_entity = self.handle()
        cdef dds_qos_t *c_qos = qos.handle()
        with nogil:
            dds_qos_get(c_entity, c_qos)
        return qos

    def inconsistent_topic_status(self):
        '''
        Return the InconstistentTopicStatus for the topic

        InconstistentTopicStatus has fields:
        total_count,
        total_count_change

        :rtype: dds.InconstistentTopicStatus
        '''
        self._check_handle()
        cdef dds_inconsistent_topic_status_t c_status
        with nogil:
            ret = dds_get_inconsistent_topic_status(<dds_entity_t>self._c_handle, &c_status)
        if ret < 0:
            raise DDSException('Failure retrieving inconsistent_topic_status', ret)
        status = InconsistentTopicStatus(c_status.total_count, c_status.total_count_change)
        return status


#Wrapper for a 'found' topic
cdef class FoundTopic(Topic):
    '''
    FoundTopic class

    A topic entity obtained via DomainParticipant.find_topic. Because of limitations
    in the underlying C99 APIs, a FoundTopic cannot be used directly to create
    data readers or data writers.

    Use the method ddutil.get_dds_classes_from_found_topic()
    dynamically create Python classes for the topic.

    Use the method register_topic on the return value of get_dds_classes_for_found_topic()
    to register a local topic that can be used for creating data readers and data writers.
    '''

    def __init__(self, DomainParticipant dp):
        self.parent = dp
        _entity_register_add(self)

cdef _FoundTopic_Init(DomainParticipant dp, dds_entity_t c_topic):
    '''
    _FoundTopic_Init(DomainParticipant dp, dds_entity_t c_topic)

    'Fake Constructor idiom' to allow passing of c_topic to FoundTopic.
    See: https://stackoverflow.com/questions/12204441/passing-c-pointer-as-argument-into-cython-function
    '''
    foundTopic = FoundTopic(dp)
    foundTopic.set_handle(c_topic)
    _entity_register_add(foundTopic)
    return foundTopic


# The publisher wrapper
cdef class Publisher(Entity):
    '''
    Publisher class

    '''
    cdef object _writers

    def __cinit__(self, DomainParticipant dp, Qos qos = None, listener = None):
        dp._check_handle()
        self._writers = []

        if not isinstance(dp, DomainParticipant):
            raise DDSException('Bad parameter')

        if qos and not isinstance(qos, Qos):
            raise DDSException('Bad parameter')

        if listener and not isinstance(listener, Listener):
            raise DDSException('Bad parameter')

        cdef dds.dds_qos_t *_c_qos = NULL
        if qos:
            _c_qos = qos.handle()

        cdef dds_publisherlistener_t *_c_listener = NULL
        if listener:
            _c_listener = &publisher_listener

        self.parent = dp
        self._listener = listener

        # enabling a listener requires three steps, done as a indivisible action:
        #   1) dds_publisher_create
        #   2) _entity_register_add
        #   3) _set_status_for_listener
        # The easiest way is to NOT release the GIL during these.
        r = dds.dds_publisher_create(dp._c_handle, &self._c_handle, _c_qos, <dds_publisherlistener_t *>_c_listener) # need_gil
        if r < 0:
            raise DDSException('Failed to create publisher', r)
        _entity_register_add(self)
        if listener:
            _set_status_for_listener(self._c_handle, listener)
        # let any triggered listeners run by yielding to other threads
        time.sleep(0)

    def create_datawriter(self, Topic topic, Qos qos = None, listener = None):
        '''create_datawriter(Topic topic, Qos qos = None, listener = None)
        Create a data writer
        :type topic: Topic
        :param topic: Topic

        :type qos: Qos
        :param qos: Qos

        :type listener: Listener
        :param listener: Listener

        :rtype: DataWriter
        '''
        writer = DataWriter(self, topic, qos, listener)
        self._writers.append(writer)
        return writer


# The subscriber wrapper
cdef class Subscriber(Entity):
    '''
    Subscriber class

    '''

    cdef object _readers

    def __cinit__(self, DomainParticipant dp, Qos qos = None, listener = None):
        dp._check_handle()
        self._readers = []

        if not isinstance(dp, DomainParticipant):
            raise DDSException('Bad parameter')

        if qos and not isinstance(qos, Qos):
            raise DDSException('Bad parameter')

        if listener and not isinstance(listener, Listener):
            raise DDSException('Bad parameter')

        cdef dds.dds_qos_t *_c_qos = NULL
        if qos:
            _c_qos = qos.handle()

        cdef dds_subscriberlistener_t *_c_listener = NULL
        if listener:
            _c_listener = &subscriber_listener

        self.parent = dp
        self._listener = listener

        # enabling a listener requires three steps, done as a indivisible action:
        #   1) dds_subscriber_create
        #   2) _entity_register_add
        #   3) _set_status_for_listener
        # The easiest way is to NOT release the GIL during these.
        r = dds.dds_subscriber_create(dp._c_handle, &self._c_handle, _c_qos, <dds_subscriberlistener_t *>_c_listener) # need_gil
        if r < 0:
            raise DDSException('Failed to create subscriber', r)
        _entity_register_add(self)
        if listener:
            _set_status_for_listener(self._c_handle, listener)
        # let any triggered listeners run by yielding to other threads
        time.sleep(0)

    def create_datareader(self, Topic topic, Qos qos = None, listener = None):
        '''create_datareader(Topic topic, Qos qos = None, listener = None)
        Create a data reader
        :type topic: Topic
        :param topic: Topic

        :type qos: Qos
        :param qos: Qos

        :type listener: Listener
        :param listener: Listener

        :rtype: DataReader
        '''
        reader = DataReader(self, topic, qos, listener)
        self._readers.append(reader)
        return reader

# The Datawriter wrapper
cdef class DataWriter(Entity):
    '''
    DataWriter class

    '''
    cdef object _typesupport
    cdef object _topic

    def __cinit__(self, Entity entity, Topic topic, Qos qos = None, listener = None):
        entity._check_handle()
        if not isinstance(entity, DomainParticipant) and not isinstance(entity, Publisher):
            raise DDSException('Bad parameter')

        if not isinstance(topic, Topic):
            raise DDSException('Bad parameter')

        if isinstance(topic, FoundTopic):
            raise DDSException('Cannot create directly data writer from FoundTopic. Use ddsutil.get_dds_classes_for_found_topic()')

        if qos and not isinstance(qos, Qos):
            raise DDSException('Bad parameter')

        if listener and not isinstance(listener, Listener):
            raise DDSException('Bad parameter')

        cdef dds.dds_qos_t *_c_qos = NULL
        if qos:
            _c_qos = qos.handle()

        cdef dds_writerlistener_t *_c_listener = NULL
        if listener:
            _c_listener = &writer_listener

        self.parent = entity
        self._topic = topic
        self._typesupport = topic._typesupport
        self._listener = listener

        # enabling a listener requires three steps, done as a indivisible action:
        #   1) dds_writer_create
        #   2) _entity_register_add
        #   3) _set_status_for_listener
        # The easiest way is to NOT release the GIL during these.
        r = dds.dds_writer_create(entity._c_handle, &self._c_handle, topic._c_handle, _c_qos, <dds_writerlistener_t *>_c_listener) # need_gil
        if r < 0:
            raise DDSException('Failed to create data writer', r)
        _entity_register_add(self)
        if listener:
            _set_status_for_listener(self._c_handle, listener)
        # let any triggered listeners run by yielding to other threads
        time.sleep(0)

    def write(self, data):
        '''write(data)
        Write data

        :type data: object
        :param data: Topic data class instance
        '''
        self._check_handle()
        cdef char *p = NULL
        b = self._typesupport._serialize(data)[:self._typesupport.size]
        p = <bytes>b
        with nogil:
            r = dds.dds_write(self._c_handle, <const void *>p)
        if r < 0:
            raise DDSException('Failed to write data', r)

    def write_ts(self, data, ts):
        '''
        Write data with a given source timestamp

        :type data: object
        :param data: data values
        :type ts: dds.DDSTime
        :param ts: a timestamp
        '''
        self._check_handle()
        cdef char *p = NULL
        b = self._typesupport._serialize(data)[:self._typesupport.size]
        p = <bytes>b
        cdef dds_time_t c_time = ts.value
        with nogil:
            r = dds.dds_write_ts(self._c_handle, <const void *>p, c_time)
        if r < 0:
            raise DDSException('Failed to write data', r)

    def dispose_instance(self, data):
        '''dispose_instance(data)
        Dispose instance

        :type data: object
        :param data: Topic data class instance
        '''
        self._check_handle()
        cdef char *p = NULL
        b = self._typesupport._serialize(data)[:self._typesupport.size]
        p = <bytes>b
        with nogil:
            r = dds.dds_instance_dispose(self._c_handle, <const void *>p)
        if r < 0:
            raise DDSException('Failed to dispose instance', r)

    def dispose_instance_ts(self, data, ts):
        '''dispose_instance_ts(data, ts)
        Dispose instance with timestamp

        :type data: object
        :param data: Topic data class instance

        :type ts: long
        :param ts: timestamp
        '''
        self._check_handle()
        cdef char *p = NULL
        b = self._typesupport._serialize(data)[:self._typesupport.size]
        p = <bytes>b

        cdef dds_time_t _c_ts = ts
        with nogil:
            r = dds.dds_instance_dispose_ts(self._c_handle, <const void *>p, _c_ts)
        if r < 0:
            raise DDSException('Failed to dispose instance', r)

    def publication_matched_status(self):
        '''
        Return the PublicationMatchedStatus of the writer

        PublicationMatchedStatus has fields:
            total_count,
            total_count_change,
            current_count,
            current_count_change,
            last_subscription_handle

        :rtype: dds.PublicationMatchedStatus
        '''
        self._check_handle()
        cdef dds_publication_matched_status_t c_status
        with nogil:
            ret = dds_get_publication_matched_status(<dds_entity_t>self._c_handle, &c_status)
        if ret < 0:
            raise DDSException('Failure retrieving publication_matched_status', ret)
        status = PublicationMatchedStatus(
            c_status.total_count,
            c_status.total_count_change,
            c_status.current_count,
            c_status.current_count_change,
            c_status.last_subscription_handle)
        return status

    def liveliness_lost_status(self):
        '''
        Return the LivelinessLostStatus of the writer

        LivelinessLostStatus has fields:
            total_count,
            total_count_change

        :rtype: dds.LivelinessLostStatus
        '''
        self._check_handle()
        cdef dds_liveliness_lost_status_t c_status
        with nogil:
            ret = dds_get_liveliness_lost_status(<dds_entity_t>self._c_handle, &c_status)
        if ret < 0:
            raise DDSException('Failure retrieving liveliness_lost_status', ret)
        status = LivelinessLostStatus(
            c_status.total_count,
            c_status.total_count_change)
        return status

    def offered_deadline_missed_status(self):
        '''
        Return the OfferedDeadlineMissedStatus of the writer

        OfferedDeadlineMissedStatus has fields:
            total_count,
            total_count_change,
            last_instance_handle

        :rtype: dds.OfferedDeadlineMissedStatus
        '''
        self._check_handle()
        cdef dds_offered_deadline_missed_status_t c_status
        with nogil:
            ret = dds_get_offered_deadline_missed_status(<dds_entity_t>self._c_handle, &c_status)
        if ret < 0:
            raise DDSException('Failure retrieving offered_deadline_missed_status', ret)
        status = OfferedDeadlineMissedStatus(
            c_status.total_count,
            c_status.total_count_change,
            c_status.last_instance_handle)
        return status

    def offered_incompatible_qos_status(self):
        '''
        Return the OfferedIncompatibleQosStatus of the writer

        OfferedIncompatibleQosStatus has fields:
            total_count,
            total_count_change,
            last_policy_id

        :rtype: dds.OfferedDeadlineMissedStatus
        '''
        self._check_handle()
        cdef dds_offered_incompatible_qos_status_t c_status
        with nogil:
            ret = dds_get_offered_incompatible_qos_status(<dds_entity_t>self._c_handle, &c_status)
        if ret < 0:
            raise DDSException('Failure retrieving offered_incompatible_qos_status', ret)
        status = OfferedIncompatibleQosStatus(
            c_status.total_count,
            c_status.total_count_change,
            QosPolicyId(c_status.last_policy_id))
        return status

    def register_instance(self, data):
        '''
        Register a data instance

        :param data: a topic data instance
        :type data: object
        :return: the instance handle
        :rtype: int
        '''
        self._check_handle()
        cdef char *p = NULL
        b = self._typesupport._serialize(data)[:self._typesupport.size]
        p = <bytes>b
        with nogil:
            r = dds.dds_instance_register(self._c_handle, <const void *>p)
        return r

    def unregister_instance(self, data=None, handle=None, timestamp=None):
        '''
        Unregister a data instance. One of data and handle must be specified.

        :param data: an optional data object
        :type data: object
        :param handle: an optional instance handle
        :type handle: int
        :param timestamp: an optional source timestamp
        :type timestamp: dds.DDSTime

        :raise DDSException: if incorrect parameters are applied

        Note: If an unregistered key ID is passed as instance data, an error is logged and not flagged as return value
        '''
        self._check_handle()
        cdef char *p = NULL
        cdef dds_instance_handle_t h = DDS_HANDLE_NIL
        cdef dds_time_t c_time
        if data is None and handle is None:
            raise DDSException('Either data or handle must be provided')
        if handle:
            h = handle
        else:
            b = self._typesupport._serialize(data)[:self._typesupport.size]
            p = <bytes>b
        if not isinstance(timestamp,DDSTime):
            with nogil:
                r = dds.dds_instance_unregister(self._c_handle, <const void *>p, h)
        else:
            c_time = timestamp.value
            with nogil:
                r = dds.dds_instance_unregister_ts(self._c_handle, <const void *>p, h, c_time)
        if r < 0:
            raise DDSException('Failure unregistering handle', r)

    def lookup_instance(self, data):
        '''
        Lookup the registered handle for the given data values

        :param data: a data object
        :type data: object
        :return: the instance handle or None
        :rtype: int
        '''
        self._check_handle()
        cdef char *p = NULL
        b = self._typesupport._serialize(data)[:self._typesupport.size]
        p = <bytes>b
        with nogil:
            h = dds.dds_instance_lookup(self._c_handle, <const void *>p)
        if h == 0:
            return None
        else:
            return h

# FIXME: cannot implement. dds_instance_get_key blows up
#     def get_key(self, handle):
#         '''
#         Accept an instance handle and return a key-value corresponding to it
#
#         :param handle: an instance handle
#         :type handle: int
#         :return: a data instance, with key-values initialized
#         :rtype: object
#         '''
#         cdef char *c_data = NULL;
#         cdef size_t sz = self._typesupport.size
#         cdef dds_instance_handle_t h = handle
#         self._check_handle()
#         print('get_key: handle = {}'.format(h))
#         print('get_key: sz = {}'.format(sz))
#         c_data = <char *>malloc(sz)
#         print('get_key: malloc''d')
#         memset(c_data, 0, sz)
#         print('get_key: memset''d')
#         try:
#             r = dds_instance_get_key(self._c_handle, h, c_data)
#             print('get_key: dds_instance_get_key')
#             if r < 0:
#                 print('get_key: dds_instance_get_key')
#                 raise DDSException('Could not get key', r)
#
#             data = self._typesupport._deserialize(c_data[:sz])
#             print('get_key: serialized')
#         finally:
#             free(c_data)


class SampleInfo:
    '''
    Base sample info class
    '''
    def __init__(self):
        pass

_Sample = namedtuple('_Sample',['data','status'])

ctypedef int (*_ReadOp)(dds_entity_t rd, void ** buf, uint32_t maxs, dds_sample_info_t * si, uint32_t mask, dds_instance_handle_t handle, dds_condition_t cond)

cdef int _op_dds_read(dds_entity_t rd, void ** buf, uint32_t maxs, dds_sample_info_t * si, uint32_t mask, dds_instance_handle_t handle, dds_condition_t cond):
    with nogil:
        return dds_read(rd, buf, maxs, si, mask)

cdef int _op_dds_take(dds_entity_t rd, void ** buf, uint32_t maxs, dds_sample_info_t * si, uint32_t mask, dds_instance_handle_t handle, dds_condition_t cond):
    with nogil:
        return dds_take(rd, buf, maxs, si, mask)

cdef int _op_dds_read_cond(dds_entity_t rd, void ** buf, uint32_t maxs, dds_sample_info_t * si, uint32_t mask, dds_instance_handle_t handle, dds_condition_t cond):
    with nogil:
        return dds_read_cond(rd, buf, maxs, si, cond)

cdef int _op_dds_take_cond(dds_entity_t rd, void ** buf, uint32_t maxs, dds_sample_info_t * si, uint32_t mask, dds_instance_handle_t handle, dds_condition_t cond):
    with nogil:
        return dds_take_cond(rd, buf, maxs, si, cond)

cdef int _op_dds_read_instance(dds_entity_t rd, void ** buf, uint32_t maxs, dds_sample_info_t * si, uint32_t mask, dds_instance_handle_t handle, dds_condition_t cond):
    with nogil:
        return dds_read_instance(rd, buf, maxs, si, handle, mask)

cdef int _op_dds_take_instance(dds_entity_t rd, void ** buf, uint32_t maxs, dds_sample_info_t * si, uint32_t mask, dds_instance_handle_t handle, dds_condition_t cond):
    with nogil:
        return dds_take_instance(rd, buf, maxs, si, handle, mask)

cdef _set_sample_info(object si, dds_sample_info_t values):
    si.sample_state = values.sample_state
    si.view_state  = values. view_state
    si.instance_state = values.instance_state
    si.valid_data = values.valid_data
    si.source_timestamp = values.source_timestamp
    si.instance_handle = values.instance_handle
    si.publication_handle = values.publication_handle
    si.disposed_generation_count = values.disposed_generation_count
    si.no_writers_generation_count = values.no_writers_generation_count
    si.sample_rank = values.sample_rank
    si.generation_rank = values.generation_rank
    si.absolute_generation_rank = values.absolute_generation_rank
    si.reception_timestamp = values.reception_timestamp

cdef list _do_read_op(TypeSupport ts, dds_entity_t rd, _ReadOp op, uint32_t n, uint32_t mask, dds_instance_handle_t handle, dds_condition_t cond):
    cdef dds.dds_sample_info_t *info = <dds.dds_sample_info_t*>malloc(n * sizeof(dds.dds_sample_info_t))
    cdef char **samples = <char**>malloc(n * sizeof(char*))
    samples[0] = NULL
    try:
        nr = op(rd, <void **>samples, n, info, mask, handle, cond)
        if nr < 0:
            raise DDSException('DDS read/take operation failed', nr)

        try:
            data = []
            for i in range(nr):
                si = SampleInfo()
                _set_sample_info(si, info[i])
                sp = ts._deserialize(samples[i][:ts.size])
                data.append(_Sample(sp, si))
        finally:
            if nr > 0:
                with nogil:
                    dds.dds_return_loan(rd, <void **>samples, n)
    finally:
        free(samples)
        free(info)

    return data

# The DataReader wrapper
cdef class DataReader(Entity):
    '''
    DataReader class

    '''
    cdef object _typesupport
    cdef object _topic

    def __cinit__(self, Entity entity, Topic topic, Qos qos=None, listener = None):
        entity._check_handle()
        if not isinstance(entity, DomainParticipant) and not isinstance(entity, Subscriber):
            raise DDSException('Bad parameter')

        if not isinstance(topic, Topic):
            raise DDSException('Bad parameter')

        if isinstance(topic, FoundTopic):
            raise DDSException('Cannot create directly data reader from FoundTopic. Use ddsutil.get_dds_classes_for_found_topic()')

        if qos and not isinstance(qos, Qos):
            raise DDSException('Bad parameter')

        if listener and not isinstance(listener, Listener):
            raise DDSException('Bad parameter')

        cdef dds.dds_qos_t *_c_qos = NULL
        if qos:
            _c_qos = qos.handle()

        self.parent = entity
        self._topic = topic
        self._typesupport = topic._typesupport
        self._listener = listener

        cdef dds_readerlistener_t *_c_listener = NULL
        if listener:
            _c_listener = &reader_listener

        # enabling a listener requires three steps, done as a indivisible action:
        #   1) dds_reader_create
        #   2) _entity_register_add
        #   3) _set_status_for_listener
        # The easiest way is to NOT release the GIL during these.
        r = dds.dds_reader_create(entity._c_handle, &self._c_handle, topic._c_handle, _c_qos, _c_listener) #need_gil
        if r < 0:
            raise DDSException('Failed to create data reader', r)
        _entity_register_add(self)
        if listener:
            _set_status_for_listener(self._c_handle, listener)
        # let any triggered listeners run by yielding to other threads
        time.sleep(0)

    def read(self, n=1, masks=DDSMaskUtil.all_samples()):
        '''read(n=1, masks=DDSMaskUtil.all_samples())
        Read samples

        :type n: int
        :param n: number of samples to read

        :type masks: list
        :param masks: list of DDS statuses

        :rtype: list
        :return: list of sample data and sample info tuple
        '''
        self._check_handle()
        mask = masks[0].value
        for i in range(1, len(masks)):
            mask = mask | masks[i].value

        return _do_read_op(self._typesupport, self.get_handle(), _op_dds_read, n, mask, 0, <void*>0)

    def take(self, n=1, masks=DDSMaskUtil.all_samples()):
        '''take(n=1, masks=DDSMaskUtil.all_samples())
        Take samples

        :type n: int
        :param n: number of samples to take

        :type masks: list
        :param masks: list of DDS statuses

        :rtype: list
        :return: list of sample data and sample info tuple
        '''
        self._check_handle()
        mask = masks[0].value
        for i in range(1, len(masks)):
            mask = mask | masks[i].value

        return _do_read_op(self._typesupport, self.get_handle(), _op_dds_take, n, mask, 0, <void*>0)

    def read_cond(self, Condition condition, int n=1):
        '''read_cond(condition, n=1)
        Read samples matching a condition

        :type n: int
        :param n: number of samples to read

        :type condition: dds.Condition
        :param condition: a ReadCondition or QueryCondition

        :rtype: list
        :return: list of sample data and sample info tuple
        '''
        self._check_handle()
        if not isinstance(condition, ReadCondition) and not isinstance(condition, QueryCondition):
            raise DDSException('Argument 1 is not of type dds.ReadCondition or dds.QueryCondition')

        return _do_read_op(self._typesupport, self.get_handle(), _op_dds_read_cond, n, 0, 0, condition._c_handle)

    def take_cond(self, Condition condition, n=1):
        '''take_cond(condition, n=1)
        Take samples matching a condition

        :type n: int
        :param n: number of samples to read

        :type condition: dds.ReadCondition
        :param condition: a read or status condition

        :rtype: list
        :return: list of sample data and sample info tuple
        '''
        self._check_handle()
        if not isinstance(condition, ReadCondition) and not isinstance(condition, QueryCondition):
            raise DDSException('Argument 1 is not of type dds.ReadCondition or dds.QueryCondition')

        return _do_read_op(self._typesupport, self.get_handle(), _op_dds_take_cond, n, 0, 0, condition._c_handle)

    def take_instance(self, handle, n=1, masks=DDSMaskUtil.all_samples()):
        '''take_instance(n=1, masks=DDSMaskUtil.all_samples())
        Take samples for a given instance handle

        :type handle: int
        :param handle: the instance handle to use

        :type n: int
        :param n: number of samples to take

        :type masks: list
        :param masks: list of DDS statuses

        :rtype: list
        :return: list of sample data and sample info tuple
        '''
        self._check_handle()
        mask = masks[0].value
        for i in range(1, len(masks)):
            mask = mask | masks[i].value

        return _do_read_op(self._typesupport, self.get_handle(), _op_dds_take_instance, n, mask, handle, <void*>0)

    def read_instance(self, handle, n=1, masks=DDSMaskUtil.all_samples()):
        '''read_instance(n=1, masks=DDSMaskUtil.all_samples())
        Read samples for a given instance handle

        :type handle: int
        :param handle: the instance handle to use

        :type n: int
        :param n: number of samples to take

        :type masks: list
        :param masks: list of DDS statuses

        :rtype: list
        :return: list of sample data and sample info tuple
        '''
        self._check_handle()
        mask = masks[0].value
        for i in range(1, len(masks)):
            mask = mask | masks[i].value

        return _do_read_op(self._typesupport, self.get_handle(), _op_dds_read_instance, n, mask, handle, <void*>0)

    def wait_for_historical_data(self, timeout = None):
        '''
        wait_for_historical_data(timeout = None)

        Wait for data reader to receive historical data, up to timeout duration

        :type timeout: DDSDuration
        :param timeout: timeout in seconds(default: infinity)

        :rtype: boolean
        :return: False if timeout exipred; True otherwise
        '''
        self._check_handle()
        if not timeout:
            timeout = DDSDuration.infinity()
        cdef dds_duration_t _c_timeout = timeout.value

        with nogil:
            r = dds.dds_reader_wait_for_historical_data(self._c_handle, _c_timeout)
        if dds.dds_err_no(r) == dds.DDS_RETCODE_OK: #gil_ok
            return True
        elif dds.dds_err_no(r) == dds.DDS_RETCODE_TIMEOUT: #gil_ok
            return False
        else:
            raise DDSException("Wait for historical data failed", r)

    def subscription_matched_status(self):
        '''
        Return the SubscriptionMatchedStatus of the reader

        SubscriptionMatchedStatus has fields:
            total_count,
            total_count_change,
            current_count,
            current_count_change,
            last_publication_handle

        :rtype: dds.SubscriptionMatchedStatus
        '''
        self._check_handle()
        cdef dds_subscription_matched_status_t c_status
        with nogil:
            ret = dds_get_subscription_matched_status(<dds_entity_t>self._c_handle, &c_status)
        if ret < 0:
            raise DDSException('Failure retrieving subscription_matched_status', ret)
        status = SubscriptionMatchedStatus(
            c_status.total_count,
            c_status.total_count_change,
            c_status.current_count,
            c_status.current_count_change,
            c_status.last_publication_handle)
        return status

    def liveliness_changed_status(self):
        '''
        Return the LivelinessChangedStatus of the reader

        LivelinessChangedStatus has fields:
            alive_count,
            not_alive_count,
            alive_count_change,
            not_alive_count_change,
            last_publication_handle

        :rtype: dds.LivelinessChangedStatus
        '''
        self._check_handle()
        cdef dds_liveliness_changed_status_t c_status
        with nogil:
            ret = dds_get_liveliness_changed_status(<dds_entity_t>self._c_handle, &c_status)
        if ret < 0:
            raise DDSException('Failure retrieving subscription_matched_status', ret)
        status = LivelinessChangedStatus(
            c_status.alive_count,
            c_status.not_alive_count,
            c_status.alive_count_change,
            c_status.not_alive_count_change,
            c_status.last_publication_handle)
        return status

    def requested_deadline_missed_status(self):
        '''
        Return the RequestedDeadlineMissedStatus of the reader

        RequestedDeadlineMissedStatus has fields:
            total_count,
            total_count_change,
            last_instance_handle

        :rtype: dds.RequestedDeadlineMissedStatus
        '''
        self._check_handle()
        cdef dds_requested_deadline_missed_status_t c_status
        with nogil:
            ret = dds_get_requested_deadline_missed_status(<dds_entity_t>self._c_handle, &c_status)
        if ret < 0:
            raise DDSException('Failure retrieving requested_deadline_missed_status', ret)
        status = RequestedDeadlineMissedStatus(
            c_status.total_count,
            c_status.total_count_change,
            c_status.last_instance_handle)
        return status

    def requested_incompatible_qos_status(self):
        '''
        Return the RequestedIncompatibleQosStatus of the reader

        RequestedIncompatibleQosStatus has fields:
            total_count,
            total_count_change,
            last_policy_id

        :rtype: dds.RequestedIncompatibleQosStatus
        '''
        self._check_handle()
        cdef dds_requested_incompatible_qos_status_t c_status
        with nogil:
            ret = dds_get_requested_incompatible_qos_status(<dds_entity_t>self._c_handle, &c_status)
        if ret < 0:
            raise DDSException('Failure retrieving requested_incompatible_qos_status', ret)
        status = RequestedIncompatibleQosStatus(
            c_status.total_count,
            c_status.total_count_change,
            QosPolicyId(c_status.last_policy_id))
        return status

    def sample_rejected_status(self):
        '''
        Return the SampleRejectedStatus of the reader

        SampleRejectedStatus has fields:
            total_count,
            total_count_change,
            last_reason,
            last_instance_handle

        :rtype: dds.SampleRejectedStatus
        '''
        self._check_handle()
        cdef dds_sample_rejected_status_t c_status
        with nogil:
            ret = dds_get_sample_rejected_status(<dds_entity_t>self._c_handle, &c_status)
        if ret < 0:
            raise DDSException('Failure retrieving sample_rejected_status', ret)
        status = SampleRejectedStatus(
            c_status.total_count,
            c_status.total_count_change,
            DDSSampleRejectedStatusKind(c_status.last_reason),
            c_status.last_instance_handle)
        return status

    def sample_lost_status(self):
        '''
        Return the SampleLostStatus of the reader

        SampleLostStatus has fields:
            total_count,
            total_count_change

        :rtype: dds.SampleLostStatus
        '''
        self._check_handle()
        cdef dds_sample_lost_status_t c_status
        with nogil:
            ret = dds_get_sample_lost_status(<dds_entity_t>self._c_handle, &c_status)
        if ret < 0:
            raise DDSException('Failure retrieving sample_lost_status', ret)
        status = SampleLostStatus(
            c_status.total_count,
            c_status.total_count_change)
        return status

    def create_readcondition(self, masks=DDSMaskUtil.all_samples()):
        '''
        Create a read condition for the data reader

        :type masks: list
        :param mask: a list of dds.DDSStateKind values
        :rtype: dds.ReadCondition
        :return: a read condition object
        '''
        return ReadCondition(self, masks)

    def create_querycondition(self, masks=DDSMaskUtil.all_samples(), expression = '', parameters=[]):
        '''
        Create a query condition for the data reader

        :type masks: list
        :param masks: a list of dds.DDSStateKind values
        :type expression: str
        :param expression: a SQL where expression
        :type parameters: list
        :param parameters: a list of str objects, representing SQL expression parameters
        :rtype: dds.QueryCondition
        :return: a query condition object
        '''
        return QueryCondition(self, masks, expression, parameters)

    def lookup_instance(self, data):
        '''
        Lookup the registered handle for the given data values

        :param data: a data object
        :type data: object
        :return: the instance handle or None
        :rtype: int
        '''
        self._check_handle()
        cdef char *p = NULL
        b = self._typesupport._serialize(data)[:self._typesupport.size]
        p = <bytes>b
        with nogil:
            h = dds.dds_instance_lookup(self._c_handle, <const void *>p)
        if h == 0:
            return None
        else:
            return h

# FIXME: cannot implement. dds_instance_get_key blows up
#     def get_key(self, handle):
#         '''
#         Accept an instance handle and return a key-value corresponding to it
#
#         :param handle: an instance handle
#         :type handle: int
#         :return: a data instance, with key-values initialized
#         :rtype: object
#         '''
#         cdef char *c_data = NULL;
#         cdef size_t sz = self._typesupport.size
#         cdef dds_instance_handle_t h = handle
#         self._check_handle()
#         print('get_key: handle = {}'.format(h))
#         print('get_key: sz = {}'.format(sz))
#         c_data = <char *>malloc(sz)
#         print('get_key: malloc''d')
#         memset(c_data, 0, sz)
#         print('get_key: memset''d')
#         try:
#             r = dds_instance_get_key(self._c_handle, h, c_data)
#             print('get_key: dds_instance_get_key')
#             if r < 0:
#                 print('get_key: dds_instance_get_key')
#                 raise DDSException('Could not get key', r)
#
#             data = self._typesupport._deserialize(c_data[:sz])
#             print('get_key: serialized')
#         finally:
#             free(c_data)

# Abstract Condition wrapper
# maintains the handle to the C99 condition
cdef class Condition:
    '''
    Abstract condition class
    '''
    cdef dds_condition_t _c_handle

    def __cinit__(self):
        self._c_handle = NULL

    cdef set_handle(self, dds_condition_t handle):
        self._c_handle = handle

    cdef dds_condition_t get_handle(self):
        return self._c_handle

    def triggered(self):
        if self._c_handle != NULL:
            return dds.dds_condition_triggered(self._c_handle) #gil_ok
        return False

# The StatusCondition wrapper
cdef class StatusCondition(Condition):
    '''
    Status condition

    :type entity: Entity
    :param entity: Source entity
    '''
    cdef object _entity
    cdef object _masks

    def __cinit__(self, Entity entity):
        self._entity = entity
        cdef dds_entity_t c_entity = entity.handle()
        with nogil:
            self._c_handle = dds_statuscondition_get(c_entity)
        if self._c_handle is NULL:
            raise DDSException('Failed to create status condition')

    @property
    def entity(self):
        return self._entity

    def enable_status(self, masks):

        self._masks = masks
        cdef uint32_t mask = masks[0].value
        for i in range(1, len(masks)):
            mask = mask | masks[i].value

        cdef dds_entity_t entity_handle = (<Entity>self._entity).get_handle()
        with nogil:
            r = dds.dds_status_set_enabled(entity_handle, mask)
        if r < 0:
            raise DDSException('Failed to attach condition', r)

    def get_enabled_status(self):
        return self._masks

# The GuardCondition wrapper
cdef class GuardCondition(Condition):
    '''
    Guard condition
    '''
    def __cinit__(self):
        self._c_handle = dds_guardcondition_create() #gil_ok
        if self._c_handle is NULL:
            raise DDSException('Failed to create guard condition')

    def trigger(self):
        ''' Trigger guard condition
        '''
        with nogil:
            dds_guard_trigger(self._c_handle)

    def reset(self):
        ''' Reset guard condition
        '''
        with nogil:
            dds_guard_reset (self._c_handle)

# The ReadCondition wrapper
cdef class ReadCondition(Condition):
    '''
    Read condition

    :type reader: DataReader
    :param reader: Source DataReader

    :type masks: list
    :param masks: list of masks (default: :meth:`.DDSMaskUtil.all_samples()`

    '''
    cdef object _reader

    def __cinit__(self, DataReader reader, masks=DDSMaskUtil.all_samples()):
        cdef uint32_t mask = masks[0].value
        for i in range(1, len(masks)):
            mask = mask | masks[i].value

        self._reader = reader
        cdef dds_entity_t c_entity = reader.get_handle()
        with nogil:
            self._c_handle = dds_readcondition_create(c_entity, mask)
        if self._c_handle is NULL:
            raise DDSException('Failed to create read condition')

# The QueryCondition wrapper
cdef class QueryCondition(Condition):
    '''
    Query condition

    :type reader: DataReader
    :param reader: Source DataReader

    :type masks: list
    :param masks: list of masks (default: :meth:`.DDSMaskUtil.all_samples()`

    :type expression: string
    :param expression: Expression

    :type parameters: list
    :param parameters: list of parameters in string

    '''
    cdef object _reader

    def __cinit__(self, DataReader reader, masks=DDSMaskUtil.all_samples(), expression = '', parameters=[]):
        self._reader = reader

        cdef uint32_t mask = masks[0].value
        for i in range(1, len(masks)):
            mask = mask | masks[i].value

        if not isinstance(expression, str):
            raise DDSException('Bad parameter')

        for p in parameters:
            if not isinstance(p, str):
                raise DDSException('Bad parameter')

        cdef uint32_t numpar = <uint32_t>len(parameters)
        cdef char **_c_parameters = NULL;
        if numpar > 0:
            _c_parameters = <char **>PyMem_Malloc(numpar * sizeof(char *))
            if _c_parameters is NULL:
                raise DDSException()
            for i in range(numpar):
                s = parameters[i].encode()
                _c_parameters[i] = s
        expression_bytes = expression.encode('ISO-8859-1')
        cdef char *c_expression = expression_bytes
        cdef dds_entity_t c_entity = reader.get_handle()
        with nogil:
            self._c_handle = dds_querycondition_create_sql(c_entity, mask, c_expression, <const char **>_c_parameters, numpar)
        PyMem_Free(_c_parameters)
        if self._c_handle is NULL:
            raise DDSException('Failed to create query condition')


# The WaitSet wrapper
cdef class WaitSet:
    '''
    WaitSet class
    '''
    cdef dds_waitset_t _c_handle
    cdef object _conditions

    def __cinit__(self):
        self._c_handle = dds_waitset_create() # gil_ok
        self._conditions = {}

    def __dealloc__(self):
        if self._c_handle is not NULL:
            dds_waitset_delete(self._c_handle) # gil_ok
        self._c_handle = NULL

    def get_conditions(self):
        '''
        Return conditions
        '''
        return self._conditions

    def attach(self, Condition cond):
        '''attach(Condition cond)
        Attach condition
        :type cond: Condition
        :param cond: condition to attach
        '''
        key = <intptr_t>cond.get_handle()
        if key is 0:
            raise DDSException('Bad parameter')
        if key in self._conditions:
            return
        cdef dds_waitset_t c_ws = self._c_handle
        cdef dds_condition_t c_cond = cond.get_handle()
        cdef dds_attach_t c_attach = c_cond
        with nogil:
            r = dds_waitset_attach(c_ws, c_cond, c_attach)
        if r < 0:
            raise DDSException('Failed to attach condition', r)
        self._conditions[key] = cond

    def detach(self, Condition cond):
        '''detach(Condition cond)
        Detach condition

        :type cond: Condition
        :param cond: condition to detach
        '''

        key = <intptr_t>cond.get_handle()
        if key is 0:
            raise DDSException('Bad parameter')

        if key not in self._conditions:
            return

        cdef dds_condition_t c_cond = cond.get_handle()
        with nogil:
            r = dds_waitset_detach(self._c_handle, c_cond)
        if r < 0:
            raise DDSException('Failed to attach condition', r)

        del self._conditions[key]

    def wait(self, timeout = DDSDuration.infinity()):
        '''wait(timeout = DDSDuration.infinity())

        Wait for any of the conditions attached to the waitset to be triggered

        :type timeout: DDSDuration
        :param timeout: timeout in seconds(default: infinity)

        :rtype: list
        :return: list of :class:`.Condition` objects that were triggered. list is empty if timeout expired
        '''

        conditions = []

        cdef dds_duration_t _c_timeout = timeout.value

        cdef dds_attach_t *_c_conditions = < dds_attach_t *>malloc(len(self._conditions) * sizeof(dds_attach_t))
        if _c_conditions is NULL:
            raise DDSException("No condition attached")

        cdef size_t condlen = len(self._conditions)
        with nogil:
            r = dds_waitset_wait(self._c_handle, _c_conditions, condlen, _c_timeout)
        if r < 0:
            raise DDSException("Wait failed", r)

        if r > 0:
            for i in range(r):
                key = <intptr_t>_c_conditions[i]
                conditions.append(self._conditions[key])
        free(_c_conditions)
        return conditions

cdef class QosProvider:
    '''
    QoS Provider class

    :type uri: string
    :param uri: uri to xml file

    :type profile: string
    :param profile: profile name
    '''

    cdef dds.dds_entity_t _qp

    # The init function takes a list of QosPolicies
    def __cinit__(self, uri, profile):
        uri_bytes = uri.encode('ISO-8859-1')
        profile_bytes = profile.encode('ISO-8859.1')
        cdef char *c_uri = uri_bytes
        cdef char *c_profile = profile_bytes
        with nogil:
            r = dds.dds_qosprovider_create(&self._qp, c_uri, c_profile)
        if r != dds.DDS_RETCODE_OK:
            raise DDSException("Qos provider failed", r)

    def __dealloc__(self):
        if self._qp is not NULL:
            dds.dds_free(self._qp) #gil_ok

    def get_participant_qos(self):
        '''
        Create participant QoS

        :rtype: Qos
        '''
        qos = Qos()
        cdef dds_qos_t *c_qos = qos.handle()
        with nogil:
            r = dds.dds_qosprovider_get_participant_qos(self._qp, c_qos, NULL)
        if r != dds.DDS_RETCODE_OK:
            raise DDSException("Failed to get participant QoS", r)
        return qos

    def get_topic_qos(self):
        '''
        Create topic QoS

        :rtype: Qos
        '''
        qos = Qos()
        cdef dds_qos_t *c_qos = qos.handle()
        with nogil:
            r = dds.dds_qosprovider_get_topic_qos(self._qp, c_qos, NULL)
        if r != dds.DDS_RETCODE_OK:
            raise DDSException("Failed to get topic QoS", r)
        return qos

    def get_publisher_qos(self):
        '''
        Create publisher QoS

        :rtype: Qos
        '''
        qos = Qos()
        cdef dds_qos_t *c_qos = qos.handle()
        with nogil:
            r = dds.dds_qosprovider_get_publisher_qos(self._qp, c_qos, NULL)
        if r != dds.DDS_RETCODE_OK:
            raise DDSException("Failed to get publisher QoS", r)
        return qos

    def get_subscriber_qos(self):
        '''
        Create subscriber QoS

        :rtype: Qos
        '''
        qos = Qos()
        cdef dds_qos_t *c_qos = qos.handle()
        with nogil:
            r = dds.dds_qosprovider_get_subscriber_qos(self._qp, c_qos, NULL)
        if r != dds.DDS_RETCODE_OK:
            raise DDSException("Failed to get subscriber QoS", r)
        return qos

    def get_writer_qos(self):
        '''
        Create writer QoS

        :rtype: Qos
        '''
        qos = Qos()
        cdef dds_qos_t *c_qos = qos.handle()
        with nogil:
            r = dds.dds_qosprovider_get_writer_qos(self._qp, c_qos, NULL)
        if r != dds.DDS_RETCODE_OK:
            raise DDSException("Failed to get writer QoS", r)
        return qos

    def get_reader_qos(self):
        '''
        Create reader QoS

        :rtype: Qos
        '''
        qos = Qos()
        cdef dds_qos_t *c_qos = qos.handle()
        with nogil:
            r = dds.dds_qosprovider_get_reader_qos(self._qp, c_qos, NULL)
        if r != dds.DDS_RETCODE_OK:
            raise DDSException("Failed to get reader QoS", r)
        return qos

def QosProfile(uri, profile):
    from warnings import warn
    warn('The class name QosProfile is deprecated. Use class QosProvider instead.' \
     + ' QosProfile will be removed in a future release.')
    return QosProvider(uri, profile)

class DDSException(Exception):
    '''
    DDS exception
    '''
    def __init__(self, message = '', code = 0):
        msg = message
        if code != 0:
            msg = message + "  Return code: " + dds.dds_err_str(code).decode('ISO-8859-1') #gil_ok
        super(DDSException, self).__init__(msg)

cdef class _SerializationHelper:

    @staticmethod
    def bytes_to_ptr(str):
        cdef char *s = <bytes>str
        cdef long long addr = <long long> s
        return addr

    @staticmethod
    def ptr_to_bytes(ptr, size = 0):
        cdef long long addr = ptr
        cdef char *s = <char *>addr
        if size != 0:
            return s[:size]
        return s

# When finding built-in topics to register locally, the packing format must be generated with
# these IDLPP generated descriptors, and not with the descriptors that come in over the wire.
cdef _builtin_topics_descriptors = {
'DCPSParticipant': '<MetaData version="1.0.0"><Module name="DDS"><TypeDef name="BuiltinTopicKey_t"><Array size="3"><Long/></Array></TypeDef><TypeDef name="octSeq"><Sequence><Octet/></Sequence></TypeDef><Struct name="UserDataQosPolicy"><Member name="value"><Type name="octSeq"/></Member></Struct><Struct name="ParticipantBuiltinTopicData"><Member name="key"><Type name="BuiltinTopicKey_t"/></Member><Member name="user_data"><Type name="UserDataQosPolicy"/></Member></Struct></Module></MetaData>',
'DCPSTopic': '<MetaData version="1.0.0"><Module name="DDS"><TypeDef name="BuiltinTopicKey_t"><Array size="3"><Long/></Array></TypeDef><Enum name="DurabilityQosPolicyKind"><Element name="VOLATILE_DURABILITY_QOS" value="0"/><Element name="TRANSIENT_LOCAL_DURABILITY_QOS" value="1"/><Element name="TRANSIENT_DURABILITY_QOS" value="2"/><Element name="PERSISTENT_DURABILITY_QOS" value="3"/></Enum><Struct name="Duration_t"><Member name="sec"><Long/></Member><Member name="nanosec"><ULong/></Member></Struct><Enum name="HistoryQosPolicyKind"><Element name="KEEP_LAST_HISTORY_QOS" value="0"/><Element name="KEEP_ALL_HISTORY_QOS" value="1"/></Enum><Enum name="LivelinessQosPolicyKind"><Element name="AUTOMATIC_LIVELINESS_QOS" value="0"/><Element name="MANUAL_BY_PARTICIPANT_LIVELINESS_QOS" value="1"/><Element name="MANUAL_BY_TOPIC_LIVELINESS_QOS" value="2"/></Enum><Enum name="ReliabilityQosPolicyKind"><Element name="BEST_EFFORT_RELIABILITY_QOS" value="0"/><Element name="RELIABLE_RELIABILITY_QOS" value="1"/></Enum><Struct name="TransportPriorityQosPolicy"><Member name="value"><Long/></Member></Struct><Enum name="DestinationOrderQosPolicyKind"><Element name="BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS" value="0"/><Element name="BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS" value="1"/></Enum><Struct name="ResourceLimitsQosPolicy"><Member name="max_samples"><Long/></Member><Member name="max_instances"><Long/></Member><Member name="max_samples_per_instance"><Long/></Member></Struct><Enum name="OwnershipQosPolicyKind"><Element name="SHARED_OWNERSHIP_QOS" value="0"/><Element name="EXCLUSIVE_OWNERSHIP_QOS" value="1"/></Enum><TypeDef name="octSeq"><Sequence><Octet/></Sequence></TypeDef><Struct name="DurabilityQosPolicy"><Member name="kind"><Type name="DurabilityQosPolicyKind"/></Member></Struct><Struct name="LifespanQosPolicy"><Member name="duration"><Type name="Duration_t"/></Member></Struct><Struct name="LatencyBudgetQosPolicy"><Member name="duration"><Type name="Duration_t"/></Member></Struct><Struct name="DeadlineQosPolicy"><Member name="period"><Type name="Duration_t"/></Member></Struct><Struct name="HistoryQosPolicy"><Member name="kind"><Type name="HistoryQosPolicyKind"/></Member><Member name="depth"><Long/></Member></Struct><Struct name="DurabilityServiceQosPolicy"><Member name="service_cleanup_delay"><Type name="Duration_t"/></Member><Member name="history_kind"><Type name="HistoryQosPolicyKind"/></Member><Member name="history_depth"><Long/></Member><Member name="max_samples"><Long/></Member><Member name="max_instances"><Long/></Member><Member name="max_samples_per_instance"><Long/></Member></Struct><Struct name="LivelinessQosPolicy"><Member name="kind"><Type name="LivelinessQosPolicyKind"/></Member><Member name="lease_duration"><Type name="Duration_t"/></Member></Struct><Struct name="ReliabilityQosPolicy"><Member name="kind"><Type name="ReliabilityQosPolicyKind"/></Member><Member name="max_blocking_time"><Type name="Duration_t"/></Member><Member name="synchronous"><Boolean/></Member></Struct><Struct name="DestinationOrderQosPolicy"><Member name="kind"><Type name="DestinationOrderQosPolicyKind"/></Member></Struct><Struct name="OwnershipQosPolicy"><Member name="kind"><Type name="OwnershipQosPolicyKind"/></Member></Struct><Struct name="TopicDataQosPolicy"><Member name="value"><Type name="octSeq"/></Member></Struct><Struct name="TopicBuiltinTopicData"><Member name="key"><Type name="BuiltinTopicKey_t"/></Member><Member name="name"><String/></Member><Member name="type_name"><String/></Member><Member name="durability"><Type name="DurabilityQosPolicy"/></Member><Member name="durability_service"><Type name="DurabilityServiceQosPolicy"/></Member><Member name="deadline"><Type name="DeadlineQosPolicy"/></Member><Member name="latency_budget"><Type name="LatencyBudgetQosPolicy"/></Member><Member name="liveliness"><Type name="LivelinessQosPolicy"/></Member><Member name="reliability"><Type name="ReliabilityQosPolicy"/></Member><Member name="transport_priority"><Type name="TransportPriorityQosPolicy"/></Member><Member name="lifespan"><Type name="LifespanQosPolicy"/></Member><Member name="destination_order"><Type name="DestinationOrderQosPolicy"/></Member><Member name="history"><Type name="HistoryQosPolicy"/></Member><Member name="resource_limits"><Type name="ResourceLimitsQosPolicy"/></Member><Member name="ownership"><Type name="OwnershipQosPolicy"/></Member><Member name="topic_data"><Type name="TopicDataQosPolicy"/></Member></Struct></Module></MetaData>',
'DCPSPublication': '<MetaData version="1.0.0"><Module name="DDS"><TypeDef name="BuiltinTopicKey_t"><Array size="3"><Long/></Array></TypeDef><Enum name="DurabilityQosPolicyKind"><Element name="VOLATILE_DURABILITY_QOS" value="0"/><Element name="TRANSIENT_LOCAL_DURABILITY_QOS" value="1"/><Element name="TRANSIENT_DURABILITY_QOS" value="2"/><Element name="PERSISTENT_DURABILITY_QOS" value="3"/></Enum><Struct name="Duration_t"><Member name="sec"><Long/></Member><Member name="nanosec"><ULong/></Member></Struct><Enum name="LivelinessQosPolicyKind"><Element name="AUTOMATIC_LIVELINESS_QOS" value="0"/><Element name="MANUAL_BY_PARTICIPANT_LIVELINESS_QOS" value="1"/><Element name="MANUAL_BY_TOPIC_LIVELINESS_QOS" value="2"/></Enum><Enum name="ReliabilityQosPolicyKind"><Element name="BEST_EFFORT_RELIABILITY_QOS" value="0"/><Element name="RELIABLE_RELIABILITY_QOS" value="1"/></Enum><Enum name="DestinationOrderQosPolicyKind"><Element name="BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS" value="0"/><Element name="BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS" value="1"/></Enum><TypeDef name="octSeq"><Sequence><Octet/></Sequence></TypeDef><Enum name="OwnershipQosPolicyKind"><Element name="SHARED_OWNERSHIP_QOS" value="0"/><Element name="EXCLUSIVE_OWNERSHIP_QOS" value="1"/></Enum><Struct name="OwnershipStrengthQosPolicy"><Member name="value"><Long/></Member></Struct><Enum name="PresentationQosPolicyAccessScopeKind"><Element name="INSTANCE_PRESENTATION_QOS" value="0"/><Element name="TOPIC_PRESENTATION_QOS" value="1"/><Element name="GROUP_PRESENTATION_QOS" value="2"/></Enum><TypeDef name="StringSeq"><Sequence><String/></Sequence></TypeDef><Struct name="DurabilityQosPolicy"><Member name="kind"><Type name="DurabilityQosPolicyKind"/></Member></Struct><Struct name="LifespanQosPolicy"><Member name="duration"><Type name="Duration_t"/></Member></Struct><Struct name="LatencyBudgetQosPolicy"><Member name="duration"><Type name="Duration_t"/></Member></Struct><Struct name="DeadlineQosPolicy"><Member name="period"><Type name="Duration_t"/></Member></Struct><Struct name="LivelinessQosPolicy"><Member name="kind"><Type name="LivelinessQosPolicyKind"/></Member><Member name="lease_duration"><Type name="Duration_t"/></Member></Struct><Struct name="ReliabilityQosPolicy"><Member name="kind"><Type name="ReliabilityQosPolicyKind"/></Member><Member name="max_blocking_time"><Type name="Duration_t"/></Member><Member name="synchronous"><Boolean/></Member></Struct><Struct name="DestinationOrderQosPolicy"><Member name="kind"><Type name="DestinationOrderQosPolicyKind"/></Member></Struct><Struct name="GroupDataQosPolicy"><Member name="value"><Type name="octSeq"/></Member></Struct><Struct name="TopicDataQosPolicy"><Member name="value"><Type name="octSeq"/></Member></Struct><Struct name="UserDataQosPolicy"><Member name="value"><Type name="octSeq"/></Member></Struct><Struct name="OwnershipQosPolicy"><Member name="kind"><Type name="OwnershipQosPolicyKind"/></Member></Struct><Struct name="PresentationQosPolicy"><Member name="access_scope"><Type name="PresentationQosPolicyAccessScopeKind"/></Member><Member name="coherent_access"><Boolean/></Member><Member name="ordered_access"><Boolean/></Member></Struct><Struct name="PartitionQosPolicy"><Member name="name"><Type name="StringSeq"/></Member></Struct><Struct name="PublicationBuiltinTopicData"><Member name="key"><Type name="BuiltinTopicKey_t"/></Member><Member name="participant_key"><Type name="BuiltinTopicKey_t"/></Member><Member name="topic_name"><String/></Member><Member name="type_name"><String/></Member><Member name="durability"><Type name="DurabilityQosPolicy"/></Member><Member name="deadline"><Type name="DeadlineQosPolicy"/></Member><Member name="latency_budget"><Type name="LatencyBudgetQosPolicy"/></Member><Member name="liveliness"><Type name="LivelinessQosPolicy"/></Member><Member name="reliability"><Type name="ReliabilityQosPolicy"/></Member><Member name="lifespan"><Type name="LifespanQosPolicy"/></Member><Member name="destination_order"><Type name="DestinationOrderQosPolicy"/></Member><Member name="user_data"><Type name="UserDataQosPolicy"/></Member><Member name="ownership"><Type name="OwnershipQosPolicy"/></Member><Member name="ownership_strength"><Type name="OwnershipStrengthQosPolicy"/></Member><Member name="presentation"><Type name="PresentationQosPolicy"/></Member><Member name="partition"><Type name="PartitionQosPolicy"/></Member><Member name="topic_data"><Type name="TopicDataQosPolicy"/></Member><Member name="group_data"><Type name="GroupDataQosPolicy"/></Member></Struct></Module></MetaData>',
'DCPSSubscription': '<MetaData version="1.0.0"><Module name="DDS"><TypeDef name="BuiltinTopicKey_t"><Array size="3"><Long/></Array></TypeDef><Enum name="DurabilityQosPolicyKind"><Element name="VOLATILE_DURABILITY_QOS" value="0"/><Element name="TRANSIENT_LOCAL_DURABILITY_QOS" value="1"/><Element name="TRANSIENT_DURABILITY_QOS" value="2"/><Element name="PERSISTENT_DURABILITY_QOS" value="3"/></Enum><Struct name="Duration_t"><Member name="sec"><Long/></Member><Member name="nanosec"><ULong/></Member></Struct><Enum name="LivelinessQosPolicyKind"><Element name="AUTOMATIC_LIVELINESS_QOS" value="0"/><Element name="MANUAL_BY_PARTICIPANT_LIVELINESS_QOS" value="1"/><Element name="MANUAL_BY_TOPIC_LIVELINESS_QOS" value="2"/></Enum><Enum name="ReliabilityQosPolicyKind"><Element name="BEST_EFFORT_RELIABILITY_QOS" value="0"/><Element name="RELIABLE_RELIABILITY_QOS" value="1"/></Enum><Enum name="OwnershipQosPolicyKind"><Element name="SHARED_OWNERSHIP_QOS" value="0"/><Element name="EXCLUSIVE_OWNERSHIP_QOS" value="1"/></Enum><Enum name="DestinationOrderQosPolicyKind"><Element name="BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS" value="0"/><Element name="BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS" value="1"/></Enum><TypeDef name="octSeq"><Sequence><Octet/></Sequence></TypeDef><Enum name="PresentationQosPolicyAccessScopeKind"><Element name="INSTANCE_PRESENTATION_QOS" value="0"/><Element name="TOPIC_PRESENTATION_QOS" value="1"/><Element name="GROUP_PRESENTATION_QOS" value="2"/></Enum><TypeDef name="StringSeq"><Sequence><String/></Sequence></TypeDef><Struct name="DurabilityQosPolicy"><Member name="kind"><Type name="DurabilityQosPolicyKind"/></Member></Struct><Struct name="TimeBasedFilterQosPolicy"><Member name="minimum_separation"><Type name="Duration_t"/></Member></Struct><Struct name="LatencyBudgetQosPolicy"><Member name="duration"><Type name="Duration_t"/></Member></Struct><Struct name="DeadlineQosPolicy"><Member name="period"><Type name="Duration_t"/></Member></Struct><Struct name="LivelinessQosPolicy"><Member name="kind"><Type name="LivelinessQosPolicyKind"/></Member><Member name="lease_duration"><Type name="Duration_t"/></Member></Struct><Struct name="ReliabilityQosPolicy"><Member name="kind"><Type name="ReliabilityQosPolicyKind"/></Member><Member name="max_blocking_time"><Type name="Duration_t"/></Member><Member name="synchronous"><Boolean/></Member></Struct><Struct name="OwnershipQosPolicy"><Member name="kind"><Type name="OwnershipQosPolicyKind"/></Member></Struct><Struct name="DestinationOrderQosPolicy"><Member name="kind"><Type name="DestinationOrderQosPolicyKind"/></Member></Struct><Struct name="GroupDataQosPolicy"><Member name="value"><Type name="octSeq"/></Member></Struct><Struct name="TopicDataQosPolicy"><Member name="value"><Type name="octSeq"/></Member></Struct><Struct name="UserDataQosPolicy"><Member name="value"><Type name="octSeq"/></Member></Struct><Struct name="PresentationQosPolicy"><Member name="access_scope"><Type name="PresentationQosPolicyAccessScopeKind"/></Member><Member name="coherent_access"><Boolean/></Member><Member name="ordered_access"><Boolean/></Member></Struct><Struct name="PartitionQosPolicy"><Member name="name"><Type name="StringSeq"/></Member></Struct><Struct name="SubscriptionBuiltinTopicData"><Member name="key"><Type name="BuiltinTopicKey_t"/></Member><Member name="participant_key"><Type name="BuiltinTopicKey_t"/></Member><Member name="topic_name"><String/></Member><Member name="type_name"><String/></Member><Member name="durability"><Type name="DurabilityQosPolicy"/></Member><Member name="deadline"><Type name="DeadlineQosPolicy"/></Member><Member name="latency_budget"><Type name="LatencyBudgetQosPolicy"/></Member><Member name="liveliness"><Type name="LivelinessQosPolicy"/></Member><Member name="reliability"><Type name="ReliabilityQosPolicy"/></Member><Member name="ownership"><Type name="OwnershipQosPolicy"/></Member><Member name="destination_order"><Type name="DestinationOrderQosPolicy"/></Member><Member name="user_data"><Type name="UserDataQosPolicy"/></Member><Member name="time_based_filter"><Type name="TimeBasedFilterQosPolicy"/></Member><Member name="presentation"><Type name="PresentationQosPolicy"/></Member><Member name="partition"><Type name="PartitionQosPolicy"/></Member><Member name="topic_data"><Type name="TopicDataQosPolicy"/></Member><Member name="group_data"><Type name="GroupDataQosPolicy"/></Member></Struct></Module></MetaData>',
'CMParticipant': '<MetaData version="1.0.0"><Module name="DDS"><TypeDef name="BuiltinTopicKey_t"><Array size="3"><Long/></Array></TypeDef><Struct name="ProductDataQosPolicy"><Member name="value"><String/></Member></Struct><Struct name="CMParticipantBuiltinTopicData"><Member name="key"><Type name="BuiltinTopicKey_t"/></Member><Member name="product"><Type name="ProductDataQosPolicy"/></Member></Struct></Module></MetaData>',
'CMPublisher': '<MetaData version="1.0.0"><Module name="DDS"><TypeDef name="BuiltinTopicKey_t"><Array size="3"><Long/></Array></TypeDef><Struct name="ProductDataQosPolicy"><Member name="value"><String/></Member></Struct><Struct name="EntityFactoryQosPolicy"><Member name="autoenable_created_entities"><Boolean/></Member></Struct><TypeDef name="StringSeq"><Sequence><String/></Sequence></TypeDef><Struct name="PartitionQosPolicy"><Member name="name"><Type name="StringSeq"/></Member></Struct><Struct name="CMPublisherBuiltinTopicData"><Member name="key"><Type name="BuiltinTopicKey_t"/></Member><Member name="product"><Type name="ProductDataQosPolicy"/></Member><Member name="participant_key"><Type name="BuiltinTopicKey_t"/></Member><Member name="name"><String/></Member><Member name="entity_factory"><Type name="EntityFactoryQosPolicy"/></Member><Member name="partition"><Type name="PartitionQosPolicy"/></Member></Struct></Module></MetaData>',
'CMSubscriber': '<MetaData version="1.0.0"><Module name="DDS"><TypeDef name="BuiltinTopicKey_t"><Array size="3"><Long/></Array></TypeDef><Struct name="ProductDataQosPolicy"><Member name="value"><String/></Member></Struct><Struct name="EntityFactoryQosPolicy"><Member name="autoenable_created_entities"><Boolean/></Member></Struct><Struct name="ShareQosPolicy"><Member name="name"><String/></Member><Member name="enable"><Boolean/></Member></Struct><TypeDef name="StringSeq"><Sequence><String/></Sequence></TypeDef><Struct name="PartitionQosPolicy"><Member name="name"><Type name="StringSeq"/></Member></Struct><Struct name="CMSubscriberBuiltinTopicData"><Member name="key"><Type name="BuiltinTopicKey_t"/></Member><Member name="product"><Type name="ProductDataQosPolicy"/></Member><Member name="participant_key"><Type name="BuiltinTopicKey_t"/></Member><Member name="name"><String/></Member><Member name="entity_factory"><Type name="EntityFactoryQosPolicy"/></Member><Member name="share"><Type name="ShareQosPolicy"/></Member><Member name="partition"><Type name="PartitionQosPolicy"/></Member></Struct></Module></MetaData>',
'CMDataWriter': '<MetaData version="1.0.0"><Module name="DDS"><TypeDef name="BuiltinTopicKey_t"><Array size="3"><Long/></Array></TypeDef><Struct name="ProductDataQosPolicy"><Member name="value"><String/></Member></Struct><Enum name="HistoryQosPolicyKind"><Element name="KEEP_LAST_HISTORY_QOS" value="0"/><Element name="KEEP_ALL_HISTORY_QOS" value="1"/></Enum><Struct name="ResourceLimitsQosPolicy"><Member name="max_samples"><Long/></Member><Member name="max_instances"><Long/></Member><Member name="max_samples_per_instance"><Long/></Member></Struct><Struct name="Duration_t"><Member name="sec"><Long/></Member><Member name="nanosec"><ULong/></Member></Struct><Struct name="HistoryQosPolicy"><Member name="kind"><Type name="HistoryQosPolicyKind"/></Member><Member name="depth"><Long/></Member></Struct><Struct name="WriterDataLifecycleQosPolicy"><Member name="autodispose_unregistered_instances"><Boolean/></Member><Member name="autopurge_suspended_samples_delay"><Type name="Duration_t"/></Member><Member name="autounregister_instance_delay"><Type name="Duration_t"/></Member></Struct><Struct name="CMDataWriterBuiltinTopicData"><Member name="key"><Type name="BuiltinTopicKey_t"/></Member><Member name="product"><Type name="ProductDataQosPolicy"/></Member><Member name="publisher_key"><Type name="BuiltinTopicKey_t"/></Member><Member name="name"><String/></Member><Member name="history"><Type name="HistoryQosPolicy"/></Member><Member name="resource_limits"><Type name="ResourceLimitsQosPolicy"/></Member><Member name="writer_data_lifecycle"><Type name="WriterDataLifecycleQosPolicy"/></Member></Struct></Module></MetaData>',
'CMDataReader': '<MetaData version="1.0.0"><Module name="DDS"><TypeDef name="BuiltinTopicKey_t"><Array size="3"><Long/></Array></TypeDef><Struct name="ProductDataQosPolicy"><Member name="value"><String/></Member></Struct><Enum name="HistoryQosPolicyKind"><Element name="KEEP_LAST_HISTORY_QOS" value="0"/><Element name="KEEP_ALL_HISTORY_QOS" value="1"/></Enum><Struct name="ResourceLimitsQosPolicy"><Member name="max_samples"><Long/></Member><Member name="max_instances"><Long/></Member><Member name="max_samples_per_instance"><Long/></Member></Struct><Struct name="Duration_t"><Member name="sec"><Long/></Member><Member name="nanosec"><ULong/></Member></Struct><Enum name="InvalidSampleVisibilityQosPolicyKind"><Element name="NO_INVALID_SAMPLES" value="0"/><Element name="MINIMUM_INVALID_SAMPLES" value="1"/><Element name="ALL_INVALID_SAMPLES" value="2"/></Enum><Struct name="UserKeyQosPolicy"><Member name="enable"><Boolean/></Member><Member name="expression"><String/></Member></Struct><Struct name="ShareQosPolicy"><Member name="name"><String/></Member><Member name="enable"><Boolean/></Member></Struct><Struct name="HistoryQosPolicy"><Member name="kind"><Type name="HistoryQosPolicyKind"/></Member><Member name="depth"><Long/></Member></Struct><Struct name="ReaderLifespanQosPolicy"><Member name="use_lifespan"><Boolean/></Member><Member name="duration"><Type name="Duration_t"/></Member></Struct><Struct name="InvalidSampleVisibilityQosPolicy"><Member name="kind"><Type name="InvalidSampleVisibilityQosPolicyKind"/></Member></Struct><Struct name="ReaderDataLifecycleQosPolicy"><Member name="autopurge_nowriter_samples_delay"><Type name="Duration_t"/></Member><Member name="autopurge_disposed_samples_delay"><Type name="Duration_t"/></Member><Member name="autopurge_dispose_all"><Boolean/></Member><Member name="enable_invalid_samples"><Boolean/></Member><Member name="invalid_sample_visibility"><Type name="InvalidSampleVisibilityQosPolicy"/></Member></Struct><Struct name="CMDataReaderBuiltinTopicData"><Member name="key"><Type name="BuiltinTopicKey_t"/></Member><Member name="product"><Type name="ProductDataQosPolicy"/></Member><Member name="subscriber_key"><Type name="BuiltinTopicKey_t"/></Member><Member name="name"><String/></Member><Member name="history"><Type name="HistoryQosPolicy"/></Member><Member name="resource_limits"><Type name="ResourceLimitsQosPolicy"/></Member><Member name="reader_data_lifecycle"><Type name="ReaderDataLifecycleQosPolicy"/></Member><Member name="subscription_keys"><Type name="UserKeyQosPolicy"/></Member><Member name="reader_lifespan"><Type name="ReaderLifespanQosPolicy"/></Member><Member name="share"><Type name="ShareQosPolicy"/></Member></Struct></Module></MetaData>',
'DCPSType': '<MetaData version="1.0.0"><Module name="DDS"><TypeDef name="DataRepresentationId_t"><Short/></TypeDef><Struct name="TypeHash"><Member name="msb"><ULongLong/></Member><Member name="lsb"><ULongLong/></Member></Struct><TypeDef name="octSeq"><Sequence><Octet/></Sequence></TypeDef><Struct name="TypeBuiltinTopicData"><Member name="name"><String/></Member><Member name="data_representation_id"><Type name="DataRepresentationId_t"/></Member><Member name="type_hash"><Type name="TypeHash"/></Member><Member name="meta_data"><Type name="octSeq"/></Member><Member name="extentions"><Type name="octSeq"/></Member></Struct></Module></MetaData>'
}

# Built-in topics that come in over the wire have the internal type name. We must swap them for
# spec name in order to find the classes in the class_dict.
cdef _builtin_topics_type_name_map = {
'kernelModule::v_participantInfo': 'DDS::ParticipantBuiltinTopicData',
'kernelModule::v_topicInfo': 'DDS::TopicBuiltinTopicData',
'kernelModule::v_publicationInfo': 'DDS::PublicationBuiltinTopicData',
'kernelModule::v_subscriptionInfo': 'DDS::SubscriptionBuiltinTopicData',
'kernelModule::v_participantCMInfo': 'DDS::CMParticipantBuiltinTopicData',
'kernelModule::v_publisherCMInfo': 'DDS::CMPublisherBuiltinTopicData',
'kernelModule::v_subscriberCMInfo': 'DDS::CMSubscriberBuiltinTopicData',
'kernelModule::v_dataWriterCMInfo': 'DDS::CMDataWriterBuiltinTopicData',
'kernelModule::v_dataReaderCMInfo': 'DDS::CMDataReaderBuiltinTopicData',
'kernelModule::v_typeInfo': 'DDS::TypeBuiltinTopicData'
}
