/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

using System;
using System.Runtime.InteropServices;

using DDS.OpenSplice.Database;
using DDS.OpenSplice.User;

namespace DDS
{
    // ----------------------------------------------------------------------
    // Types & Pre-defined values
    // ----------------------------------------------------------------------


    public struct Property
    {
        private string _name;
        private string _value;

        public Property(string name, string val)
        {
            _name = name;
            _value = val;
        }

        public string Name
        {
            get { return _name; }
            set { _name = value; }
        }

        public string Value
        {
            get { return _value; }
            set { _value = value; }
        }
    }

    /// <summary>
    /// Duration represents a time interval.
    /// </summary>
    /// Duration can be converted to and from Durations expressed in milliseconds
    /// (or other units) as integer types.
    ///
    [StructLayout(LayoutKind.Sequential)]
    public struct Duration
    {
        [MarshalAs(UnmanagedType.I4)]
        private int sec;

        /// <summary>
        /// Gets the seconds part of the Duration.
        /// </summary>
        public int Sec
        {
            get { return sec; }
            set { sec = value; }
        }

        [MarshalAs(UnmanagedType.U4)]
        private uint nanosec;
        /// <summary>
        /// Gets the nanoseconds part of the Duration.
        /// </summary>
        public uint NanoSec
        {
            get { return nanosec; }
            set { nanosec = value; }
        }

        /** @cond */
        public const int InfiniteSec = 0x7fffffff;
        public const uint InfiniteNanoSec = 0x7fffffff;

        public const int ZeroSec = 0;
        public const uint ZeroNanoSec = 0;
        /** @endcond */

        /// <summary>
        /// Create an infinite Duration.
        /// </summary>
        public static readonly Duration Infinite = new Duration(InfiniteSec, InfiniteNanoSec);

        /// <summary>
        /// Create a Duration elapsing zero seconds.
        /// </summary>
        public static readonly Duration Zero = new Duration(ZeroSec, ZeroNanoSec);

        /// <summary>
        /// Create a Duration based on TimeSpan.
        /// </summary>
        /// <remarks>
        /// <param name="value">TimeSpan value to convert to Duration</param>
        /// </remarks>
        public static Duration FromTimeSpan(TimeSpan value)
        {
            return FromMilliseconds(value.TotalMilliseconds);
        }

        /// <summary>
        /// Create a Duration based on milliseconds.
        /// </summary>
        /// <remarks>
        /// <param name="value">Milliseconds value to convert to Duration</param>
        /// </remarks>
        public static Duration FromMilliseconds(double value)
        {
            return new Duration((int)(value / 1000),
                (uint)((Math.Abs(value) % 1000) * 1000000));
        }

        /// <summary>
        /// Create a Duration elapsing a specific amount of time.
        /// </summary>
        /// <remarks>
        /// <param name="seconds">Amount of seconds for the Duration.</param>
        /// <param name="nanoSeconds">Amount of nanoseconds for the Duration.</param>
        /// </remarks>
        public Duration(int seconds, uint nanoSeconds)
        {
            sec = seconds;
            nanosec = nanoSeconds;
        }

        /// <summary>
        /// Test this Duration is equal to the provided Duration.
        /// </summary>
        /// <remarks>
        /// <param name="obj">Duration to compare.</param>
        /// <returns>true if the Duration is equal.</returns
        /// </remarks>
        public override bool Equals(object obj)
        {
            if (!(obj is Duration))
                return false;

            return ((Duration)obj).sec == sec && ((Duration)obj).nanosec == nanosec;
        }

        /// <summary>
        /// Calculates hash of the Duration.
        /// </summary>
        /// <remarks>
        /// <returns>Hash code.</returns
        /// </remarks>
        public override int GetHashCode()
        {
            return (int)nanosec;
        }

        /// <summary>
        /// Test if two Durations are equal.
        /// </summary>
        /// <remarks>
        /// <param name="duration1">Duration to compare.</param>
        /// <param name="duration2">Duration to compare.</param>
        /// <returns>true if the Durations are equal.</returns
        /// </remarks>
        public static bool operator ==(Duration duration1, Duration duration2)
        {
            return (duration1.sec == duration2.sec && duration1.nanosec == duration2.nanosec);
        }

        /// <summary>
        /// Test if two Durations are not equal.
        /// </summary>
        /// <remarks>
        /// <param name="duration1">Duration to compare.</param>
        /// <param name="duration2">Duration to compare.</param>
        /// <returns>true if the Durations are not equal.</returns
        /// </remarks>
        public static bool operator !=(Duration duration1, Duration duration2)
        {
            return !(duration1 == duration2);
        }

        /// <summary>
        /// Converts Duration to a string.
        /// </summary>
        /// <remarks>
        /// <returns>String containing the duration.</returns
        /// </remarks>
        public override string ToString()
        {
            return string.Format("{0}.{1}", sec, nanosec);
        }

        /** @cond */
        internal c_time DatabaseTime
        {
            get
            {
                c_time dbTime = new c_time();
                dbTime.seconds = sec;
                dbTime.nanoseconds = nanosec;
                return dbTime;
            }
        }
        /** @endcond */

        private const long OsDurationInfinite = 0x7fffffffffffffff;
        private const uint OsDurationSecond = 1000000000;
        /** @cond */
        internal long OsDuration
        {
            get
            {
                long d;
                if (this == Infinite) {
                    d = OsDurationInfinite;
                } else {
                    d = (long)((long)sec*OsDurationSecond+(long)nanosec);
                }
                return d;
            }
            set
            {
                if (value == OsDurationInfinite) {
                    this = Infinite;
                } else {
                    sec = (int)(value / OsDurationSecond);
                    nanosec = (uint)(value % OsDurationSecond);
                }
            }
        }
        /** @endcond */

    }

    /// <summary>
    /// Time represents a time value.
    /// </summary>
    /// Time can be converted to and from Times expressed in milliseconds (or other units)
    /// as integer types.
    ///
    [StructLayout(LayoutKind.Sequential)]
    public struct Time
    {
        [MarshalAs(UnmanagedType.I4)]
        private int sec;
        /// <summary>
        /// Gets the seconds part of the Time.
        /// </summary>
        public int Sec
        {
            get { return sec; }
            set { sec = value; }
        }

        [MarshalAs(UnmanagedType.U4)]
        private uint nanosec;
        /// <summary>
        /// Gets the nanoseconds part of the Time.
        /// </summary>
        public uint NanoSec
        {
            get { return nanosec; }
            set { nanosec = value; }
        }

        /** @cond */
        public const int InvalidSec = -1;
        public const uint InvalidNanoSec = 0xffffffff;
        public const int ZeroSec = 0;
        public const uint ZeroNanoSec = 0;
        /** @endcond */

        /// <summary>
        /// Create an infinite Time.
        /// </summary>
        public static readonly Time Invalid = new Time(InvalidSec, InvalidNanoSec);

        /**
         * Create an zero Time.
         */
        public static readonly Time Zero = new Time(ZeroSec, ZeroNanoSec);

        /**
         * Create a Time with the current time.
         */
        public static readonly Time Current = new Time(InvalidSec, InvalidNanoSec-1);

        /// <summary>
        /// Create a Time struct based on a DateTime value.
        /// </summary>
        /// <remarks>
        /// <param name="value">DateTime value to convert to Duration</param>
        /// </remarks>
        public static Time FromDateTime(DateTime value)
        {
            DateTime origin = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc);
            TimeSpan diff = value - origin;
            return new Time((int)Math.Floor(diff.TotalSeconds), (uint)(diff.Milliseconds * 1000000));
        }

        /// <summary>
        /// Create a DateTime struct based on the Time value.
        /// </summary>
        public DateTime ToDatetime()
        {
            DateTime origin = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc);
            return origin.AddSeconds(sec).AddTicks(nanosec / 100);
        }

        private const ulong OsTimeInvalid = 0xffffffffffffffff;
        private const uint OsTimeSecond = 1000000000;
        /** @cond */
        internal os_timeW OsTimeW
        {
            get
            {
                os_timeW t = new os_timeW();
                if ((this == Current) ||
                    (this == Invalid)) {
                    t.wt = OsTimeInvalid;
                } else {
                    t.wt = (ulong)((long)this.Sec * OsTimeSecond + (long)this.NanoSec);
                }
                return t;
            }
            set
            {
                if (value.wt == OsTimeInvalid) {
                    this = Invalid;
                } else {
                    this.sec = (int)(value.wt / OsTimeSecond);
                    this.nanosec = (uint)(value.wt % OsTimeSecond);
                }
            }
        }
        /** @endcond */

        /// <summary>
        /// Create a Time based on seconds and nanoseconds.
        /// </summary>
        /// <remarks>
        /// <param name="_sec">Amount of seconds for the Time.</param>
        /// <param name="_nanosec">Amount of nanoseconds for the Time.</param>
        /// </remarks>
        public Time(int _sec, uint _nanosec)
        {
            sec = _sec;
            nanosec = _nanosec;
        }

        /// <summary>
        /// Test this Time is equal to the provided Time.
        /// </summary>
        /// <remarks>
        /// <param name="obj">Time to compare.</param>
        /// <returns>true if the Time is equal.</returns
        /// </remarks>
        public override bool Equals(object obj)
        {
            if (!(obj is Time))
                return false;

            return ((Time)obj).sec == sec && ((Time)obj).nanosec == nanosec;
        }

        /// <summary>
        /// Calculates hash of the Time.
        /// </summary>
        /// <remarks>
        /// <returns>Hash code.</returns
        /// </remarks>
        public override int GetHashCode()
        {
            return (int)nanosec;
        }

        /// <summary>
        /// Test if two Times are equal.
        /// </summary>
        /// <remarks>
        /// <param name="left">Time to compare.</param>
        /// <param name="right">Time to compare.</param>
        /// <returns>true if the Times are equal.</returns
        /// </remarks>
        public static bool operator ==(Time left, Time right)
        {
            return (left.sec == right.sec && left.nanosec == right.nanosec);
        }

        /// <summary>
        /// Test if two Times are not equal.
        /// </summary>
        /// <remarks>
        /// <param name="left">Time to compare.</param>
        /// <param name="right">Time to compare.</param>
        /// <returns>true if the Times not are equal.</returns
        /// </remarks>
        public static bool operator !=(Time left, Time right)
        {
            return !(left == right);
        }

        /// <summary>
        /// Test one Time is before another Time.
        /// </summary>
        /// <remarks>
        /// <param name="left">Time to compare.</param>
        /// <param name="right">Time to compare.</param>
        /// <returns>true if left < right.</returns
        /// </remarks>
        public static bool operator >(Time left, Time right)
        {
            return (left.sec > right.sec) || (left.sec == right.sec && left.nanosec > right.nanosec);
        }

        /// <summary>
        /// Test one Time is after another Time.
        /// </summary>
        /// <remarks>
        /// <param name="left">Time to compare.</param>
        /// <param name="right">Time to compare.</param>
        /// <returns>true if left > right.</returns
        /// </remarks>
        public static bool operator <(Time left, Time right)
        {
            return right > left;
        }

        /// <summary>
        /// Test one Time is equal of after another Time.
        /// </summary>
        /// <remarks>
        /// <param name="left">Time to compare.</param>
        /// <param name="right">Time to compare.</param>
        /// <returns>true if left >= right.</returns
        /// </remarks>
        public static bool operator >=(Time left, Time right)
        {
            return (left.sec >= right.sec) || (left.sec == right.sec && left.nanosec >= right.nanosec);
        }

        /// <summary>
        /// Test one Time is equal of before another Time.
        /// </summary>
        /// <remarks>
        /// <param name="left">Time to compare.</param>
        /// <param name="right">Time to compare.</param>
        /// <returns>true if left <= right.</returns
        /// </remarks>
        public static bool operator <=(Time left, Time right)
        {
            return right >= left;
        }

        /// <summary>
        /// Converts Time to a string.
        /// </summary>
        /// <remarks>
        /// <returns>String containing the time.</returns
        /// </remarks>
        public override string ToString()
        {
            return string.Format("{0}.{1}", sec, nanosec);
        }
    }

    /// <summary>
    /// PlaceHolder for a domain ID.
    /// </summary>
    /// <remarks>
    /// This marshals as an int or 32-bit integer
    /// </remarks>
    public struct DomainId
    {
        private int value;

        /// <summary>
        /// Representation of the default domain ID.
        /// </summary>
        /// <remarks>
        /// This domain ID causes the creation of a DomainParticipant to
        /// use the domain ID that is provided within the configuration
        /// or 0 when no ID was provided (which is the default).
        /// </remarks>
        public static readonly DomainId Default = new DomainId(0x7fffffff);

        /// <summary>
        /// Representation of an invalid domain ID.
        /// </summary>
        public static readonly DomainId Invalid = new DomainId(-1);

        /// <summary>
        /// Create an DomainId based on an integer.
        /// </summary>
        /// <remarks>
        /// <param name="id">integer representation of the domainId.</param>
        /// </remarks>
        public DomainId(int id)
        {
            value = id;
        }

        /// <summary>
        /// Test this DomainId is equal to the provided DomainId.
        /// </summary>
        /// <remarks>
        /// <param name="obj">DomainId to compare.</param>
        /// <returns>true if the DomainId is equal.</returns
        /// </remarks>
        public override bool Equals(object obj)
        {
            if (!(obj is DomainId))
                return false;

            return ((DomainId)obj).value == value;
        }

        /// <summary>
        /// Calculates hash of the DomainId.
        /// </summary>
        /// <remarks>
        /// <returns>Hash code.</returns
        /// </remarks>
        public override int GetHashCode()
        {
            return (int)value;
        }

        /// <summary>
        /// Converts the DomainId to a long.
        /// </summary>
        /// <remarks>
        /// <returns>DomainId as a long.</returns
        /// </remarks>
        public long ToInt32()
        {
            return value;
        }

        /// <summary>
        /// Test if two DomainIds are equal.
        /// </summary>
        /// <remarks>
        /// <param name="id1">DomainId to compare.</param>
        /// <param name="id2">DomainId to compare.</param>
        /// <returns>true if the DomainIds are equal.</returns
        /// </remarks>
        public static bool operator ==(DomainId id1, DomainId id2)
        {
            return (id1.value == id2.value);
        }

        /// <summary>
        /// Test if two DomainIds are not equal.
        /// </summary>
        /// <remarks>
        /// <param name="id1">DomainId to compare.</param>
        /// <param name="id2">DomainId to compare.</param>
        /// <returns>true if the DomainIds are not equal.</returns
        /// </remarks>
        public static bool operator !=(DomainId id1, DomainId id2)
        {
            return (id1.value != id2.value);
        }

        /// <summary>
        /// Implicit conversion from int to DomainId.
        /// </summary>
        /// <remarks>
        /// <param name="id">integer to convert</param>
        /// <returns>new DomainId.</returns
        /// </remarks>
        public static implicit operator DomainId(int id)
        {
            return new DomainId(id);
        }

        /// <summary>
        /// Implicit conversion from DomainId to int.
        /// </summary>
        /// <remarks>
        /// <param name="id">DomainId to convert</param>
        /// <returns>DomainId as int.</returns
        /// </remarks>
        public static implicit operator int(DomainId id)
        {
            return id.value;
        }

        /// <summary>
        /// Converts DomainId to a string.
        /// </summary>
        /// <remarks>
        /// <returns>String containing the DomainId.</returns
        /// </remarks>
        public override string ToString()
        {
            return ToString(null);
        }

        /// <summary>
        /// Converts DomainId to a string with possible integer format.
        /// </summary>
        /// <remarks>
        /// <returns>String containing the DomainId.</returns
        /// </remarks>
        public string ToString(string format)
        {
            return value.ToString(format);
        }
    }


    /// <summary>
    /// Class to hold the handle associated with in sample instance.
    /// </summary>
    public struct InstanceHandle
    {
        private long value;

        /// <summary>
        /// Create an nil instance handle.
        /// </summary>
        public static readonly InstanceHandle Nil = new InstanceHandle(0);

        /// <summary>
        /// Constructor to create an instance handle that represents the long.
        /// </summary>
        /// <remarks>
        /// <param name="handleVal">handleVal long to convert into an InstanceHandle.</param>
        /// </remarks>
        public InstanceHandle(long handleVal)
        {
            value = handleVal;
        }

        /// <summary>
        /// Test this InstanceHandle is equal to the provided InstanceHandle.
        /// </summary>
        /// <remarks>
        /// <param name="obj">InstanceHandle to compare.</param>
        /// <returns>true if the InstanceHandle is equal.</returns
        /// </remarks>
        public override bool Equals(object obj)
        {
            if (!(obj is InstanceHandle))
                return false;

            return ((InstanceHandle)obj).value == value;
        }

        /// <summary>
        /// Calculates hash of the InstanceHandle.
        /// </summary>
        /// <remarks>
        /// <returns>Hash code.</returns
        /// </remarks>
        public override int GetHashCode()
        {
            return (int)value;
        }

        /// <summary>
        /// Converts the InstanceHandle into an long.
        /// </summary>
        /// <remarks>
        /// <returns>InstanceHandle as long.</returns
        /// </remarks>
        public long ToInt64()
        {
            return value;
        }

        /// <summary>
        /// Test if two InstanceHandles are equal.
        /// </summary>
        /// <remarks>
        /// <param name="handle1">InstanceHandle to compare.</param>
        /// <param name="handle2">InstanceHandle to compare.</param>
        /// <returns>true if the InstanceHandles are equal.</returns
        /// </remarks>
        public static bool operator ==(InstanceHandle handle1, InstanceHandle handle2)
        {
            return (handle1.value == handle2.value);
        }

        /// <summary>
        /// Test if two InstanceHandles are not equal.
        /// </summary>
        /// <remarks>
        /// <param name="handle1">InstanceHandle to compare.</param>
        /// <param name="handle2">InstanceHandle to compare.</param>
        /// <returns>true if the InstanceHandles are not equal.</returns
        /// </remarks>
        public static bool operator !=(InstanceHandle handle1, InstanceHandle handle2)
        {
            return (handle1.value != handle2.value);
        }

        /// <summary>
        /// Implicit conversion from long to InstanceHandle.
        /// </summary>
        /// <remarks>
        /// <returns>long as InstanceHandle.</returns
        /// </remarks>
        public static implicit operator InstanceHandle(long handleVal)
        {
            return new InstanceHandle(handleVal);
        }

        /// <summary>
        /// Implicit conversion from InstanceHandle to long.
        /// </summary>
        /// <remarks>
        /// <returns>InstanceHandle as long.</returns
        /// </remarks>
        public static implicit operator long(InstanceHandle handle)
        {
            return handle.value;
        }

        /// <summary>
        /// Converts InstanceHandle to a string.
        /// </summary>
        /// <remarks>
        /// <returns>String containing the InstanceHandle.</returns
        /// </remarks>
        public override string ToString()
        {
            return ToString(null);
        }

        /// <summary>
        /// Converts InstanceHandle to a string with possible integer format.
        /// </summary>
        /// <remarks>
        /// <returns>String containing the InstanceHandle.</returns
        /// </remarks>
        public string ToString(string format)
        {
            return value.ToString(format);
        }
    }

    /// <summary>
    /// Constant to represent the Unlimited length value.
    /// </summary>
    /// <remarks>
    /// <returns>InstanceHandle as long.</returns
    /// </remarks>
    public struct Length
    {
        /// <summary>
        /// Unlimited length constant.
        /// </summary>
        public const int Unlimited = -1;
    }



    // ----------------------------------------------------------------------
    // Status structs
    // ----------------------------------------------------------------------

    /// <summary>
    /// Another topic exists with the same name but different characteristics.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public class InconsistentTopicStatus
    {
        /// <summary>
        /// Total cumulative count of all inconsistent topics detected.
        /// </summary>
        public int TotalCount;

        /// <summary>
        /// The incremental number of inconsistent topics since the last time
        /// the listener was called or the status was read.
        /// </summary>
        public int TotalCountChange;
    }


    /// <summary>
    /// Indicates that a sample has been lost.
    /// </summary>
    /// <remarks>
    /// A sample never reached the DataReader queue. The following events can lead to a sample lost status:
    /// <list type="bullet">
    /// <item>
    /// When the Presentation QoS is used with coherentAccess set to true and ResourceLimits Qos is
    /// active a sample lost can occur when all resources are consumed by incomplete transactions.
    /// In order to prevent deadlocks the the current transaction is dropped which causes a SampleLost event.
    /// </item>
    /// <item>
    /// When the DestinationOrder QoS is set to BY_SOURCE_TIMESTAMP It can happen that older data is
    /// inserted after newer data is already taken by the DataReader. In this case the older data is
    /// not inserted into the DataReader and a Sample Lost is reported.
    /// </item>
    /// <item>
    /// When the networking service detects a gap between sequence numbers of incoming data it will
    /// report a Sample Lost event.
    /// </item>
    /// </list>
    /// </remarks>
    [StructLayout(LayoutKind.Sequential)]
    public class SampleLostStatus
    {
        /// <summary>
        /// Total cumulative count of all samples lost across of instances of data
        /// published under the Topic.
        /// </summary>
        public int TotalCount;

        /// <summary>
        /// The incremental number of samples lost since the last time the listener
        /// was called or the status was read.
        /// </summary>
        public int TotalCountChange;
    }


    /// <summary>
    /// A received sample was rejected.
    /// </summary>
    /// <remarks>
    /// This can happen when the ResourceLimits Qos is active and the History determined
    /// by History QoS of the DataReader is full.
    /// </remarks>
    [StructLayout(LayoutKind.Sequential)]
    public class SampleRejectedStatus
    {
        /// <summary>
        /// Total cumulative count of samples rejected by the DataReader.
        /// </summary>
        public int TotalCount;

        /// <summary>
        /// The incremental number of samples rejected since the last time the
        /// listener was called or the status was read.
        /// </summary>
        public int TotalCountChange;

        /// <summary>
        /// Reason for rejecting the last sample rejected. If no samples have been
        /// rejected, the reason is the special value NOT_REJECTED.
        /// </summary>
        public SampleRejectedStatusKind LastReason;

        /// <summary>
        /// Handle to the instance being updated by the last sample that was
        /// rejected.
        /// </summary>
        public InstanceHandle LastInstanceHandle;
    }

    /// <summary>
    /// The liveliness of the DataWriter set by the QoS policy is not respected and DataReader
    /// entities will consider the DataWriter as "inactive"
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public class LivelinessLostStatus
    {
        /// <summary>
        /// Total cumulative number of times that a previously-alive DataWriter
        /// became 'not alive' due to a failure to actively signal its liveliness within
        /// its offered liveliness period.
        /// </summary>
        /// <remarks>
        /// This count does not change when an
        /// already not alive DataWriter simply remains not alive for another
        /// liveliness period.
        /// </remarks>
        public int TotalCount;

        /// <summary>
        /// The change in total_count since the last time the listener was called or
        /// the status was read.
        /// </summary>
        public int TotalCountChange;
    }

    /// <summary>
    /// The liveliness of one or more DataWriter that were writing instances
    /// have become "active" or "inactive"
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public class LivelinessChangedStatus
    {
        /// <summary>
        /// The total number of currently active DataWriters that write the Topic
        /// read by the DataReader.
        /// </summary>
        /// <remarks>
        /// This count increases when a newly-matched
        /// DataWriter asserts its liveliness for the first time or when a DataWriter
        /// previously considered to be not alive reasserts its liveliness. The count
        /// decreases when a DataWriter considered alive fails to assert its
        /// liveliness and becomes not alive, whether because it was deleted
        /// normally or for some other reason.
        /// </remarks>
        public int AliveCount;

        /// <summary>
        /// The total count of currently DataWriters that write the Topic read by
        /// the DataReader that are no longer asserting their liveliness.
        /// </summary>
        /// <remarks>
        /// This count
        /// increases when a DataWriter considered alive fails to assert its
        /// liveliness and becomes not alive for some reason other than the normal
        /// deletion of that DataWriter. It decreases when a previously not alive
        /// DataWriter either reasserts its liveliness or is deleted normally.
        /// </remarks>
        public int NotAliveCount;

        /// <summary>
        /// The change in the alive_count since the last time the listener was
        /// called or the status was read.
        /// </summary>
        public int AliveCountChange;

        /// <summary>
        /// The change in the not_alive_count since the last time the listener was
        /// called or the status was read.
        /// </summary>
        public int NotAliveCountChange;

        /// <summary>
        /// Handle to the last DataWriter whose change in liveliness caused this
        /// status to change.
        /// </summary>
        public InstanceHandle LastPublicationHandle;
    }

    /// <summary>
    /// The deadline QoS set by the DataWriter was not respected for a specific instance
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public class OfferedDeadlineMissedStatus
    {
        /// <summary>
        /// Total cumulative number of offered deadline periods elapsed during
        /// which a DataWriter failed to provide data.
        /// </summary>
        /// <remarks>
        /// Missed deadlines
        /// accumulate; that is, each deadline period the total_count will be
        /// incremented by one.
        /// </remarks>
        public int TotalCount;

        /// <summary>
        /// The change in total_count since the last time the listener was called or
        /// the status was read.
        /// </summary>
        public int TotalCountChange;

        /// <summary>
        /// Handle to the last instance in the DataWriter for which an offered
        /// deadline was missed.
        /// </summary>
        public InstanceHandle LastInstanceHandle;
    }

    /// <summary>
    /// The deadline that the DataReader was expecting through its QoS policy
    /// was not respected for a specific instance.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public class RequestedDeadlineMissedStatus
    {
        /// <summary>
        /// Total cumulative number of missed deadlines detected for any instance
        /// read by the DataReader.
        /// </summary>
        /// <remarks>
        /// Missed deadlines accumulate; that is, each
        /// deadline period the total_count will be incremented by one for each
        /// instance for which data was not received.
        /// </remarks>
        public int TotalCount;

        /// <summary>
        /// The incremental number of deadlines detected since the last time the
        /// listener was called or the status was read.
        /// </summary>
        public int TotalCountChange;

        /// <summary>
        /// Handle to the last instance in the DataReader for which a deadline was
        /// detected.
        /// </summary>
        public InstanceHandle LastInstanceHandle;
    }

    /// <summary>
    /// A QoS policy value incompatible with the available DataReader
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public class OfferedIncompatibleQosStatus
    {
        /// <summary>
        /// Total cumulative number of times the concerned DataWriter
        /// discovered a DataReader for the same Topic with a requested QoS that
        /// is incompatible with that offered by the DataWriter.
        /// </summary>
        public int TotalCount;

        /// <summary>
        /// The change in total_count since the last time the listener was called or
        /// the status was read.
        /// </summary>
        public int TotalCountChange;

        /// <summary>
        /// The PolicyId of one of the policies that was found to be
        /// incompatible the last time an incompatibility was detected.
        /// </summary>
        public QosPolicyId LastPolicyId;

        /// <summary>
        /// A list containing for each policy the total number of times that the
        /// concerned DataWriter discovered a DataReader for the same Topic
        /// with a requested QoS that is incompatible with that offered by the
        /// DataWriter.
        /// </summary>
        public QosPolicyCount[] Policies;
    }

    /// <summary>
    /// A QoS policy requested is incompatible with the offered QoS policy by DataWriter.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public class RequestedIncompatibleQosStatus
    {
        /// <summary>
        /// Total cumulative number of times the concerned DataReader
        /// discovered a DataWriter for the same Topic with an offered QoS that
        /// was incompatible with that requested by the DataReader.
        /// </summary>
        public int TotalCount;

        /// <summary>
        /// The change in total_count since the last time the listener was called or
        /// the status was read.
        /// </summary>
        public int TotalCountChange;

        /// <summary>
        /// The QosPolicyId of one of the policies that was found to be
        /// incompatible the last time an incompatibility was detected.
        /// </summary>
        public QosPolicyId LastPolicyId;

        /// <summary>
        /// A list containing for each policy the total number of times that the
        /// concerned DataReader discovered a DataWriter for the same Topic
        /// with an offered QoS that is incompatible with that requested by the
        /// DataReader.
        /// </summary>
        public QosPolicyCount[] Policies;
    }

    /// <summary>
    /// The DataWriter has found a DataReader that matches the Topic and has
    /// compatible QoS or ceased to be matched with a DataReader.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public class PublicationMatchedStatus
    {
        /// <summary>
        /// Total cumulative count the concerned DataWriter discovered a
        /// "match" with a DataReader.
        /// </summary>
        /// <remarks>
        /// That is, it found a DataReader for the
        /// same Topic with a requested QoS that is compatible with that offered
        /// by the DataWriter.
        /// </remarks>
        public int TotalCount;

        /// <summary>
        /// The change in total_count since the last time the listener was called or
        /// the status was read.
        /// </summary>
        public int TotalCountChange;

        /// <summary>
        /// The number of DataReaders currently matched to the concerned
        /// DataWriter.
        /// </summary>
        public int CurrentCount;

        /// <summary>
        /// The change in current_count since the last time the listener was called
        /// or the status was read.
        /// </summary>
        public int CurrentCountChange;

        /// <summary>
        /// Handle to the last DataReader that matched the DataWriter causing the
        /// status to change.
        /// </summary>
        public long LastSubscriptionHandle;
    }

    /// <summary>
    /// The DataReader has found a DataWriter that matches the Topic and has
    /// compatible QoS or ceased to be matched with a DataWriter.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public class SubscriptionMatchedStatus
    {
        /// <summary>
        /// Total cumulative count the concerned DataReader discovered a
        /// "match" with a DataWriter.
        /// <summary>
        /// </summary>
        /// That is, it found a DataWriter for the same
        /// Topic with a requested QoS that is compatible with that offered by the
        /// DataReader.
        /// </remarks>
        public int TotalCount;

        /// <summary>
        /// The change in total_count since the last time the listener was called or
        /// the status was read.
        /// </summary>
        public int TotalCountChange;

        /// <summary>
        /// The number of DataWriters currently matched to the concerned
        /// DataReader.
        /// </summary>
        public int CurrentCount;

        /// <summary>
        /// The change in current_count since the last time the listener was called
        /// or the status was read.
        /// </summary>
        public int CurrentCountChange;

        /// <summary>
        /// Handle to the last DataWriter that matched the DataReader causing the
        /// status to change.
        /// </summary>
        public long LastPublicationHandle;
    }

    /// <summary>
    /// Counts for a QosPolicy the number of imcompatible readers or writers.
    /// </summary>
    /// <remarks>
    /// The QosPolicyCount object shows, for a QosPolicy, the total number of
    /// times that the concerned DataWriter discovered a DataReader for the
    /// same Topic and a requested DataReaderQos that is incompatible with
    /// the one offered by the DataWriter.
    /// </remarks>
    public struct QosPolicyCount
    {
        /// <summary>
        /// The QosPolicyId
        /// </summary>
        public QosPolicyId PolicyId;

        /// <summary>
        /// The Count
        /// </summary>
        public int Count;
    }



    // ----------------------------------------------------------------------
    // Qos & Policies
    // ----------------------------------------------------------------------

    public struct QosPolicyName
    {
        public const string UserData = "UserData";
        public const string Durability = "Durability";
        public const string Presentation = "Presentation";
        public const string Deadline = "Deadline";
        public const string LatencyBudget = "LatencyBudget";
        public const string Ownership = "Ownership";
        public const string OwnershipStrength = "OwnershipStrength";
        public const string Liveliness = "Liveliness";
        public const string TimeBasedFilter = "TimeBasedFilter";
        public const string Partition = "Partition";
        public const string Reliability = "Reliability";
        public const string DestinationOrder = "DestinationOrder";
        public const string History = "History";
        public const string ResourceLimits = "ResourceLimits";
        public const string EntityFactory = "EntityFactory";
        public const string WriterDataLifecycle = "WriterDataLifecycle";
        public const string ReaderDataLifecycle = "ReaderDataLifecycle";
        public const string TopicData = "TopicData";
        public const string GroupData = "GroupData";
        public const string TransportPriority = "TransportPriority";
        public const string Lifespan = "Lifespan";
        public const string DurabilityService = "DurabilityService";
        public const string Scheduling = "Scheduling";
    }

#if DOXYGEN_FOR_CS
/*
 * The above compile switch is never (and must never) be defined in normal compilation.
 *
 * QoS related classes are part of the generated code for builtin topics.
 * They are repeated here for easy documentation generation.
 */

    /**
     * \copydoc DCPS_QoS_UserData
     */
    [StructLayout(LayoutKind.Auto, Size = 200)]
    public struct UserDataQosPolicy
    {
        public byte[] Value;
    }

    /**
     * \copydoc DCPS_QoS_TopicData
     */
    public struct TopicDataQosPolicy
    {
        public byte[] Value;
    }

    /**
     * \copydoc DCPS_QoS_GroupData
     */
    public struct GroupDataQosPolicy
    {
        public byte[] Value;
    }

    /**
     * \copydoc DCPS_QoS_TransportPriority
     */
    public struct TransportPriorityQosPolicy
    {
        public int Value;
    }

    /**
     * \copydoc DCPS_QoS_Lifespan
     */
    public struct LifespanQosPolicy
    {
        public Duration Duration;
    }

    /**
     * \copydoc DCPS_QoS_Durability
     */
    public struct DurabilityQosPolicy
    {
        public DurabilityQosPolicyKind Kind;
    }

    /**
     * \copydoc DCPS_QoS_Presentation
     */
    public struct PresentationQosPolicy
    {
        public PresentationQosPolicyAccessScopeKind AccessScope;
        public bool CoherentAccess;
        public bool OrderedAccess;
    }

    /**
     * \copydoc DCPS_QoS_Deadline
     */
    public struct DeadlineQosPolicy
    {
        public Duration Period;
    }

    /**
     * \copydoc DCPS_QoS_LatencyBudget
     */
    public struct LatencyBudgetQosPolicy
    {
        public Duration Duration;
    }

    /**
     * \copydoc DCPS_QoS_Ownership
     */
    public struct OwnershipQosPolicy
    {
        public OwnershipQosPolicyKind Kind;
    }

    /**
     * \copydoc DCPS_QoS_OwnershipStrength
     */
    public struct OwnershipStrengthQosPolicy
    {
        public int Value;
    }

    /**
     * \copydoc DCPS_QoS_Liveliness
     */
    public struct LivelinessQosPolicy
    {
        public LivelinessQosPolicyKind Kind;
        public Duration LeaseDuration;
    }

    /**
     * \copydoc DCPS_QoS_TimeBasedFilter
     */
    public struct TimeBasedFilterQosPolicy
    {
        public Duration MinimumSeparation;
    }

    /**
     * \copydoc DCPS_QoS_Partition
     */
    public struct PartitionQosPolicy
    {
        public string[] Name;
    }

    /**
     * \copydoc DCPS_QoS_Reliability
     */
    public struct ReliabilityQosPolicy
    {
        public ReliabilityQosPolicyKind Kind;
        public Duration MaxBlockingTime;
        /**
         * @note Proprietary policy attribute.
         *
         * Specifies whether a DataWriter should wait for
         * acknowledgements by all connected DataReaders that also have set a
         * synchronous Reliability QosPolicy.
         */
        public bool synchronous;
    }

    /**
     * \copydoc DCPS_QoS_DestinationOrder
     */
    public struct DestinationOrderQosPolicy
    {
        public DestinationOrderQosPolicyKind Kind;
    }

    /**
     * \copydoc DCPS_QoS_History
     */
    public struct HistoryQosPolicy
    {
        public HistoryQosPolicyKind Kind;
        public int Depth;
    }

    /**
     * \copydoc DCPS_QoS_ResourceLimits
     */
    public struct ResourceLimitsQosPolicy
    {
        public int MaxSamples;
        public int MaxInstances;
        public int MaxSamplesPerInstance;
    }

    /**
     * \copydoc DCPS_QoS_EntityFactory
     */
    public struct EntityFactoryQosPolicy
    {
        [MarshalAs(UnmanagedType.U1)]
        public bool AutoEnableCreatedEntities;
    }

    /// <summary>
    /// This QosPolicy allows sharing of entities by multiple processes or threads.
    /// </summary>
    /// <remarks>
    /// @note Proprietary QoS Policy to share a DataReader between multiple processes.
    /// <para>
    /// When the policy is enabled, the data distribution service will try to look up an existing
    /// entity that matches the name supplied in the Share QosPolicy. A new entity will only
    /// be created if a shared entity registered under the specified name doesn’t exist yet.
    /// </para><para>
    /// Shared Readers can be useful for implementing algorithms like the worker pattern,
    /// where a single shared reader can contain samples representing different tasks that
    /// may be processed in parallel by separate processes. In this algorithm each processes
    /// consumes the task it is going to perform (i.e. it takes the sample representing that
    /// task), thus preventing other processes from consuming and therefore performing the
    /// same task.
    /// </para><para>
    /// <b><i>Entities can only be shared between processes if OpenSplice is running in
    /// federated mode, because it requires shared memory to communicate between the
    /// different processes.</i></b>
    /// </para><para>
    /// By default, the Share QosPolicy is not used and enable is false. Name must be set
    /// to a valid string for the Share QosPolicy to be valid when enable is set to true.
    /// This QosPolicy is applicable to DataReader and Subscriber entities, and cannot be
    /// modified after the DataReader or Subscriber is enabled. Note that a DataReader can
    /// only be shared if its Subscriber is also shared.
    /// </para>
    /// </remarks>
    public struct ShareQosPolicy
    {
        public string Name;
        public bool Enable;
    }

    /**
     * \copydoc DCPS_QoS_WriterDataLifecycle
     */
    public struct WriterDataLifecycleQosPolicy
    {
        [MarshalAs(UnmanagedType.U1)]
        public bool AutoDisposeUnregisteredInstances;
        /// <summary>
        /// Specifies the duration after which the DataWriter will automatically
        /// remove a sample from its history during periods in which its
        /// Publisher is suspended.
        /// </summary>
        /// <remarks>
        /// @note Proprietary policy attribute.
        ///
        /// This duration is calculated based on the source timestamp of the written
        /// sample. By default the duration value is set to DURATION_INFINITE and
        /// therefore no automatic purging of samples occurs.
        /// </remarks>
        public Duration AutopurgeSuspendedSamplesDelay;
        /// <summary>
        /// Specifies the duration after which the DataWriter will automatically
        /// unregister an instance after the application wrote a sample for it
        /// and no further action is performed on the same instance by this
        /// DataWriter afterwards.
        /// </summary>
        /// <remarks>
        /// @note Proprietary policy attribute.
        ///
        /// This means that when the application writes a new sample for this instance,
        /// the duration is recalculated from that action onwards. By default the
        /// duration value is DURATION_INFINITE and therefore no automatic unregistration
        /// occurs.
        /// </remarks>
        public Duration AutounregisterInstanceDelay;
    }

    /**
     * \copydoc DCPS_QoS_ReaderDataLifecycle
     */
    public struct ReaderDataLifecycleQosPolicy
    {
        public Duration AutoPurgeNoWriterSamplesDelay;
        public Duration AutoPurgeDisposedSamplesDelay;
        /// <summary>
        /// Determines whether all samples in the DataReader will be purged automatically
        /// when all data is disposed of the Topic that is associated with the DataReader.
        /// </summary>
        /// <remarks>
        /// @note Proprietary policy attribute.
        ///
        /// If this attribute set to true, no more samples will exist in the DataReader
        /// after the dispose_all_data has been processed. Because all samples are purged,
        /// no data available events will be notified to potential Listeners or Conditions
        /// that are set for the DataReader. If this attribute is set to false, the
        ///dispose_all_data behaves as if each individual instance was disposed separately.
        /// </remarks>
        public bool AutopurgeDisposeAll;
        /// <summary>
        /// Enable/disable the InvalidSampleVisibility attribute.
        /// </summary>
        /// <remarks>
        /// @note Proprietary policy attribute.
        /// </remarks>
        public bool EnableInvalidSamples;
        /// <summary>
        /// Insert dummy samples if no data sample is available, to notify readers of an
        /// instance state change.
        /// </summary>
        /// <remarks>
        /// @note Proprietary policy attribute.
        ///
        /// By default the value is MinimumInvalidSamples.
        /// </remarks>
        public InvalidSampleVisibilityQosPolicy InvalidSampleVisibility;
    }

    /// <summary>
    /// This Proprietary QosPolicy allows the DataReader to define it's own set of keys on,
    /// the data potentially different from the keys defined on the topic.
    /// </summary>
    /// <remarks>
    /// @note Proprietary QoS Policy.
    /// <para>
    /// By using the SubscriptionKey QosPolicy, a DataReader can force its own key-list
    /// definition on data samples. The consequences are that the DataReader will
    /// internally keep track of instances based on its own key list, instead of the key list
    /// dictated by the Topic.
    /// </para><para>
    /// Operations that operate on instances or instance handles, such as
    /// lookup_instance or get_key_value, respect the alternative key-list and work
    /// as expected. However, since the mapping of writer instances to reader instances is
    /// no longer trivial (one writer instance may now map to more than one matching
    /// reader instance and vice versa), a writer instance will no longer be able to fully
    /// determine the lifecycle of its matching reader instance, nor the value its
    /// view_state and instance_state.
    /// </para><para>
    /// In fact, by diverting from the conceptual 1 – 1 mapping between writer instance and
    /// reader instance, the writer can no longer keep an (empty) reader instance ALIVE by
    /// just refusing to unregister its matching writer instance. That means that when a
    /// reader takes all samples from a particular reader instance, that reader instance will
    /// immediately be removed from the reader’s administration. Any subsequent
    /// reception of a message with the same keys will re-introduce the instance into the
    /// reader administration, setting its view_state back to NEW. Compare this to the
    /// default behaviour, where the reader instance will be kept alive as long as the writer
    /// does not unregister it. That causes the view_state in the reader instance to remain
    /// NOT_NEW, even if the reader has consumed all of its samples prior to receiving an
    /// update.
    /// </para><para>
    /// Another consequence of allowing an alternative keylist is that events that are
    /// communicated by invalid samples (i.e. samples that have only initialized their
    /// keyfields) may no longer be interpreted by the reader to avoid situations in which
    /// uninitialized non-keyfields are treated as keys in the alternative keylist. This
    /// effectively means that all invalid samples (e.g. unregister messages and both
    /// implicit and explicit dispose messages) will be skipped and can no longer affect the
    /// instance_state, which will therefore remain ALIVE. The only exceptions to this
    /// are the messages that are transmitted explicitly using the
    /// DataWriter writedispose call, which always includes a full and
    /// valid sample and can therefore modify the instance_state to
    /// NOT_ALIVE_DISPOSED.
    /// </para><para>
    /// By default, the SubscriptionKeyQosPolicy is not used because use_key_list is
    /// set to false.
    /// </para><para>
    /// This QosPolicy is applicable to a DataReader only, and cannot be changed after the
    /// DataReader is enabled.
    /// </para>
    /// </remarks>
    public struct SubscriptionKeyQosPolicy
    {
        public bool UseKeyList;
        public string[] KeyList;
    }

    /// <summary>
    /// This Proprietary QosPolicy represents the SubscriptionKey QosPolicy in
    /// the proprietary builtin topic CMDataReader.
    /// </summary>
    /// <remarks>
    /// @note Proprietary QoS Policy.
    /// This QosPolicy is internaaly used in the proprietary builtin topic CMDataReader.
    /// It is a representation of the SubscriptionKey QosPolicy which is part of the
    /// DataReaderQos.
    /// <seealso cref="SubscriptionKeyQosPolicy">
    /// <seealso cref="CMDataReaderBuiltinTopicData">
    /// </remarks>
    public struct UserKeyQosPolicy
    {
        public bool Enable;
        public string Expression;
    }

    /// <summary>
    /// Proprietary QoS Policy for automatically remove samples from the DataReader after
    /// a specified timeout.
    /// </summary>
    /// <remarks>
    /// @note Proprietary QoS Policy.
    /// <para>
    /// This QosPolicy is similar to the Lifespan QosPolicy (applicable to Topic and
    /// DataWriter), but limited to the DataReader on which the QosPolicy is applied. The
    /// data is automatically removed from the DataReader if it has not been taken yet after
    /// the lifespan duration expires. The duration of the ReaderLifespan is added to the
    /// insertion time of the data in the DataReader to determine the expiry time.
    /// </para><para>
    /// When both the ReaderLifespan QosPolicy and a DataWriter’s
    /// Lifespan QosPolicy are applied to the same data, only the earliest expiry time is
    /// taken into account.
    /// </para><para>
    /// By default, the ReaderLifespan QosPolicy is not used and use_lifespan is false.<br>
    /// The duration is set to DDS.Duration.Infinite.
    /// </para>
    /// </remarks>
    public struct ReaderLifespanQosPolicy
    {
        public bool UseLifespan;
        public Duration Duration;
    }

    /**
     * \copydoc DCPS_QoS_DurabilityService
     */
    public struct DurabilityServiceQosPolicy
    {
        public Duration ServiceCleanupDelay;
        public HistoryQosPolicyKind HistoryKind;
        public int HistoryDepth;
        public int MaxSamples;
        public int MaxInstances;
        public int MaxSamplesPerInstance;
    }

    /// <summary>
    /// The scheduling policy which indicates if the scheduling priority is
    /// relative or absolute.
    /// </summary>
    /// <remarks>
    /// @note Proprietary scope class used in @ref SchedulingQosPolicy.
    /// </remarks>
    public struct SchedulingClassQosPolicy
    {
        public SchedulingClassQosPolicyKind Kind;
    }

    /// <summary>
    /// The scheduling priority.
    /// </summary>
    /// <remarks>
    /// @note Proprietary scope class used in @ref SchedulingQosPolicy.
    /// </remarks>
    public struct SchedulingPriorityQosPolicy
    {
        public SchedulingPriorityQosPolicyKind Kind;
    }

    /// <summary>
    /// Indicate how invalid samples are handled.
    /// </summary>
    /// <remarks>
    /// @note Proprietary scope class used in @ref ReaderDataLifecycleQosPolicy.
    /// </remarks>
    public struct InvalidSampleVisibilityQosPolicy
    {
        public InvalidSampleVisibilityQosPolicyKind Kind;
    };

    /// <summary>
    /// Proprietary QoS Policy for specifying the scheduling class and priorities of
    /// the DDS related threads.
    /// </summary>
    /// <remarks>
    /// @note Proprietary QoS Policy.
    /// <para>
    /// <b><i>Note that some scheduling parameters may
    /// not be supported by the underlying Operating System, or that you may need special
    /// privileges to select particular settings. Refer to the documentation of your OS for
    /// more details on this subject.</i></b>
    /// </para><para>
    /// Although the behaviour of the scheduling_class is highly dependent on the
    /// underlying OS, in general it can be said that when running in a Timesharing class
    /// your thread will have to yield execution to other threads of equal priority regularly.
    /// In a Realtime class your thread normally runs until completion, and can only be
    /// pre-empted by higher priority threads. Often the highest range of priorities is not
    /// accessible through a Timesharing Class.
    /// </para><para>
    /// The scheduling_priority_kind determines whether the specified
    /// scheduling_priority should be interpreted as an absolute priority, or whether it
    /// should be interpreted relative to the priority of its creator, in this case the priority of
    /// the thread that created the DomainParticipant.
    /// </para>
    /// </remarks>
    public struct SchedulingQosPolicy
    {
        public SchedulingClassQosPolicy SchedulingClass;
        public SchedulingPriorityQosPolicy SchedulingPriorityKind;
        public int SchedulingPriority;
    }

    /// <summary>
    /// Proprietary QoS Policy for specifying internal product information which is used
    /// by a number of proprietary builtin topics.
    /// </summary>
    /// <remarks>
    /// @note Proprietary QoS Policy.
    /// <para>
    /// This Policy is part of many proprietary builtin topics:
    /// <list>
    /// <item>DDS.CMParticipantBuiltinTopicData</item>
    /// <item>DDS.CMPublisherBuiltinTopicData</item>
    /// <item>DDS.CMSubscriberBuiltinTopicData</item>
    /// <item>DDS.CMDataWriterBuiltinTopicData</item>
    /// <item>DDS.CMDataReaderBuiltinTopicData</item>
    /// </list>
    /// </para><para>
    /// The policy contains product information of the product that created
    /// the related entity. The data will be formatted in XML. It is mostly
    /// used by OpenSplice tools to display extended information about the
    /// discovered entities.
    /// </para>
    /// </remarks>
    public struct ProductDataQosPolicy
    {
        public string Value;
    };




    /// <summary>
    /// This class provides the basic mechanism for an application to specify Quality of
    /// Service attributes for a DomainParticipantFactory.
    /// </summary>
    /// <remarks>
    /// The QosPolicy cannot be set at creation time, since the
    /// DomainParticipantFactory is a pre-existing object that can only be obtained
    /// with the DomainParticipantFactory.get_instance operation. Therefore its QosPolicy
    /// is initialized to a default value according to the following table:
    /// <list type="table">
    /// <listheader>
    /// <term>QosPolicy</term>
    /// <term>Attribute</term>
    /// <term><Default Value</term>
    /// </listheader>
    /// <item><term>DomainParticipantQos.EntityFactory</term><term>EntityFactoryQosPolicy.AutoEnableCreatedEntities</term><term>true</term></item>
    /// </list>
    /// After creation the QosPolicy can be modified with the set_qos operation on the
    /// DomainParticipantFactory, which takes the DomainParticipantFactoryQos class as a parameter.
    ///
    /// @see @ref DCPS_QoS
    /// </remarks>
    public struct DomainParticipantFactoryQos
    {
        /// <summary>
        /// Specifies whether a just created DomainParticipant should be enabled.
        /// </summary>
        /// <remarks>
        /// @ref DCPS_QoS_EntityFactory "Entity Factory Qos"
        ///
        public EntityFactoryQosPolicy EntityFactory;
    }


    /// <summary>
    /// This class provides the basic mechanism for an application to specify Quality of
    /// Service attributes for an IDomainParticipant.
    /// </summary>
    /// <remarks>
    /// An IDomainParticipant will spawn different threads for different purposes:
    /// <list type="bullet">
    /// <item>
    /// A listener thread is spawned to perform the callbacks to all Listener objects
    /// attached to the various Entities contained in the IDomainParticipant. The
    /// scheduling parameters for this thread can be specified in the
    /// ListenerScheduling attribute of the DomainParticipantQos.
    /// </item>
    /// <item>
    /// A watchdog thread is spawned to report the the Liveliness of all Entities
    /// contained in the IDomainParticipant whose LivelinessQosPolicyKind in
    /// their DDS.LivelinessQosPolicy is set to AutomaticLivelinessQos. The
    /// scheduling parameters for this thread can be specified in the
    /// WatchdogScheduling attribute of the DomainParticipantQos.
    /// </item>
    /// </list><para>
    /// A QosPolicy can be set when the IDomainParticipant is created with the
    /// DomainParticipantFactory.CreateParticipant operation (or modified with the
    /// SetQos operation). Both operations take the DomainParticipantQos object as a parameter.
    /// There may be cases where several policies are in conflict. Consistency checking is performed
    /// each time the policies are modified when they are being created and, in case they are
    /// already enabled, via the SetQos operation.
    /// </para><para>
    /// Some QosPolicy have “immutable” semantics meaning that they can only be
    /// specified either at IDomainParticipant creation time or prior to calling the Enable
    /// operation on the IDomainParticipant.
    /// </para><para>
    /// The initial value of the default DomainParticipantQos in the DomainParticipantFactory
    /// are given in the following table:
    /// <list type="table">
    /// <listheader>
    /// <term>QosPolicy</term>
    /// <term>Attribute</term>
    /// <term><Default Value</term>
    /// </listheader>
    /// <item><term>DomainParticipantQos.UserData</term><term>UserDataQosPolicy.Value</term><term>empty</term></item>
    /// <item><term>DomainParticipantQos.EntityFactory</term><term>EntityFactoryQosPolicy.AutoEnableCreatedEntities</term><term>true</term></item>
    /// <item><term rowspan="3">DomainParticipantQos.ListenerScheduling</term><term>SchedulingQosPolicy.SchedulingClass</term><term>ScheduleDefault</term></item>
    /// <item><term>SchedulingQosPolicy.SchedulingPriority</term><term>0</term></item>
    /// <item><term>SchedulingQosPolicy.SchedulingPriorityKind</term><term>PriorityRelative</term></item>
    /// </list></para>
    ///
    /// @see @ref DCPS_QoS
    /// </remarks>
    public struct DomainParticipantQos
    {
        /// <summary>
        /// Used to attach additional information to the IDomainParticipant
        /// </summary>
        /// <remarks>
        /// @ref DCPS_QoS_UserData "User Data QoS"
        ///
        public UserDataQosPolicy UserData;
        /// <summary>
        /// Specifies whether a just created IEntity should be enabled.
        /// </summary>
        /// <remarks>
        /// @ref DCPS_QoS_EntityFactory "Entity Factory QoS"
        ///
        public EntityFactoryQosPolicy EntityFactory;
        /// <summary>
        /// Specifies the scheduling parameters used to create the watchdog thread.
        /// </summary>
        /// <remarks>
        /// @note Proprietary Policy
        ///
        public SchedulingQosPolicy WatchdogScheduling;
        /// <summary>
        /// Specifies the scheduling parameters used to create the listener thread.
        /// </summary>
        /// <remarks>
        /// @note Proprietary Policy
        ///
        public SchedulingQosPolicy ListenerScheduling;
    }



    /// <summary>
    /// This struct provides the basic mechanism for an application to specify Quality of
    /// Service attributes for a Topic.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A QosPolicy can be set when the ITopic is created with the IDomainParticipant.CreateTopic
    /// operation (or modified with the SetQos operation). Both operations take the
    /// TopicQos object as a parameter. There may be cases where several policies are in
    /// conflict. Consistency checking is performed each time the policies are modified
    /// when they are being created and, in case they are already enabled, via the SetQos
    /// operation.
    /// </para><para>
    /// Some QosPolicy have “immutable” semantics meaning that they can only be
    /// specified either at ITopic creation time or prior to calling the Enable operation on
    /// the ITopic.
    /// </para><para>
    /// The initial value of the default TopicQos in the IDomainParticipant are given in
    /// the following table:
    /// <list type="table">
    /// <listheader><term>QosPolicy</term><term>Attribute</term><term>Default Value</term></listheader>
    /// <item><term>TopicQos.TopicData</term><term>TopicDataQosPolicy.Value</term><term>empty</term></item>
    /// <item><term>TopicQos.Durability</term><term>DurabilityQosPolicy.Kind</term><term>Volatile</term></item>
    /// <item><term rowspan="6">TopicQos.DurabilityService</term><term>DurabilityServiceQosPolicy.HistoryKind</term><term>KeepLastHistoryQos</term></item>
    /// <item><term>DurabilityServiceQosPolicy.HistoryDepth</term><term>1</term></item>
    /// <item><term>DurabilityServiceQosPolicy.MaxInstances</term><term>Length.Unlimited</term></item>
    /// <item><term>DurabilityServiceQosPolicy.MaxSamples</term><term>Length.Unlimited</term></item>
    /// <item><term>DurabilityServiceQosPolicy.MaxSamplesPerInstance</term><term>Length.Unlimited</term></item>
    /// <item><term>DurabilityServiceQosPolicy.ServiceCleanupDelay</term><term>Duration.Zero</term></item>
    /// <item><term>TopicQos.Deadline</term><term>DeadlineQosPolicy.Period</term><term>Duration.Infinite</term></item>
    /// <item><term>TopicQos.LatencyBudget</term><term>LatencyBudgetQosPolicy.Duration</term><term>Duration.Zero</term></item>
    /// <item><term rowspan="2">TopicQos.Liveliness</term><term>LivelinessQosPolicy.Kind</term><term>AutomaticLivelinessQos</term></item>
    /// <item><term>LivelinessQosPolicy.LeaseDuration</term><term>Duration.Infinite</term></item>
    /// <item><term rowspan="3">TopicQos.Reliability</term><term>ReliabilityQosPolicy.Kind</term><term>BestEffortReliabilityQos</term></item>
    /// <item><term>ReliabilityQosPolicy.MaxBlockingTime</term><term>100 ms</term></item>
    /// <item><term>ReliabilityQosPolicy.synchronous</term><term>false</term></item>
    /// <item><term>TopicQos.DestinationOrder</term><term>DestinationOrderQosPolicy.Kind</term><term>ByReceptionTimestampDestinationOrderQos</term></item>
    /// <item><term rowspan="2">TopicQos.History</term><term>HistoryQosPolicy.Kind</term><term>KeepLastHistoryQos</term></item>
    /// <item><term>HistoryQosPolicy.Depth</term><term>1</term></item>
    /// <item><term rowspan="3">TopicQos.ResourceLimits</term><term>ResourceLimitsQosPolicy.MaxInstances</term><term>Length.Unlimited</term></item>
    /// <item><term>ResourceLimitsQosPolicy.MaxSamples</term><term>Length.Unlimited</term></item>
    /// <item><term>ResourceLimitsQosPolicy.MaxSamplesPerInstance</term><term>Length.Unlimited</term></item>
    /// <item><term>TopicQos.TransportPriority</term><term>TransportPriorityQosPolicy.Value</term><term>0</term></item>
    /// <item><term>TopicQos.Lifespan</term><term>LifespanQosPolicy.Duration</term><term>Duration.Infinite</term></item>
    /// <item><term>TopicQos.Ownership</term><term>OwnershipQosPolicy.Kind</term><term>SharedOwnershipQos</term></item>
    /// </list>
    ///
    /// @see @ref DCPS_QoS
    /// </para>
    /// </remarks>
    public struct TopicQos
    {
        /// <summary>
        /// Used to attach additional information to the Topic.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_TopicData "Topic Data QoS"
        /// </remarks>
        public TopicDataQosPolicy TopicData;
        /// <summary>
        /// Specifies whether the data should be stored for late joining readers.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_Durability "Durability QoS"
        /// </remarks>
        public DurabilityQosPolicy Durability;
        /// <summary>
        /// Specifies the behaviour of the “transient/persistent service” of the Data Distribution System
        /// regarding Transient and Persistent Topic instances.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_DurabilityService "DurabilityService QoS"
        /// </remarks>
        public DurabilityServiceQosPolicy DurabilityService;
        /// <summary>
        /// Specifies the period within which a new sample is expected or written.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_Deadline "Deadline QoS"
        /// </remarks>
        public DeadlineQosPolicy Deadline;
        /// <summary>
        /// Used by the Data Distribution Service for optimization.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_LatencyBudget "LatencyBudget QoS"
        /// </remarks>
        public LatencyBudgetQosPolicy LatencyBudget;
        /// <summary>
        /// Specifies the way the liveliness of the Topic is asserted to the Data Distribution Service.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_Liveliness "Liveliness QoS"
        /// </remarks>
        public LivelinessQosPolicy Liveliness;
        /// <summary>
        /// Specifies the reliability of the data distribution.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_Reliability "Reliability QoS"
        /// </remarks>
        public ReliabilityQosPolicy Reliability;
        /// <summary>
        /// Specifies the order in which the DataReader timely orders the data.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_DestinationOrder "DestinationOrder QoS"
        /// </remarks>
        public DestinationOrderQosPolicy DestinationOrder;
        /// <summary>
        /// Specifies how samples should be stored.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_History "History QoS"
        /// </remarks>
        public HistoryQosPolicy History;
        /// <summary>
        /// Specifies the maximum amount of resources to be used.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_ResourceLimits "ResourceLimits QoS"
        /// </remarks>
        public ResourceLimitsQosPolicy ResourceLimits;
        /// <summary>
        /// Specifies a priority hint for the underlying transport layer.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_TransportPriority "TransportPriority QoS"
        /// </remarks>
        public TransportPriorityQosPolicy TransportPriority;
        /// <summary>
        /// Specifies the maximum duration of validity of the data written by a DataWriter.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_Lifespan "Lifespan QoS"
        /// </remarks>
        public LifespanQosPolicy Lifespan;
        /// <summary>
        /// Specifies whether a DataWriter exclusively owns an instance.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_Ownership "Ownership QoS"
        /// </remarks>
        public OwnershipQosPolicy Ownership;
    }



    /// <summary>
    /// This struct provides the basic mechanism for an application to specify Quality of
    /// Service attributes for an IDataWriter.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A QosPolicy can be set when the IDataWriter is created with the IPublisher.CreateDataWriter
    /// operation (or modified with the SetQos operation). Both operations take the
    /// DataWriterQos object as a parameter. There may be cases where several policies are in
    /// conflict. Consistency checking is performed each time the policies are modified
    /// when they are being created and, in case they are already enabled, via the SetQos
    /// operation.
    /// </para><para>
    /// Some QosPolicy have “immutable” semantics meaning that they can only be
    /// specified either at IDataWriter creation time or prior to calling the Enable operation on
    /// the IDataWriter.
    /// </para><para>
    /// The initial value of the default DataWriterQos in the IPublisher are given in
    /// the following table:
    /// <list type="table">
    /// <listheader><term>QosPolicy</term><term>Attribute</term><term>Default Value</term></listheader>
    /// <item><term>DataWriterQos.UserData</term><term>UserDataQosPolicy.Value</term><term>empty</term></item>
    /// <item><term>DataWriterQos.Durability</term><term>DurabilityQosPolicy.Kind</term><term>Volatile</term></item>
    /// <item><term>DataWriterQos.Deadline</term><term>DeadlineQosPolicy.Period</term><term>Duration.Infinite</term></item>
    /// <item><term>DataWriterQos.LatencyBudget</term><term>LatencyBudgetQosPolicy.Duration</term><term>Duration.Zero</term></item>
    /// <item><term rowspan="2">DataWriterQos.Liveliness</term><term>LivelinessQosPolicy.Kind</term><term>AutomaticLivelinessQos</term></item>
    /// <item><term>LivelinessQosPolicy.LeaseDuration</term><term>Duration.Infinite</term></item>
    /// <item><term rowspan="3">DataWriterQos.Reliability</term><term>ReliabilityQosPolicy.Kind</term><term>BestEffortReliabilityQos</term></item>
    /// <item><term>ReliabilityQosPolicy.MaxBlockingTime</term><term>100 ms</term></item>
    /// <item><term>ReliabilityQosPolicy.synchronous</term><term>false</term></item>
    /// <item><term>DataWriterQos.DestinationOrder</term><term>DestinationOrderQosPolicy.Kind</term><term>ByReceptionTimestampDestinationOrderQos</term></item>
    /// <item><term rowspan="2">DataWriterQos.History</term><term>HistoryQosPolicy.Kind</term><term>KeepLastHistoryQos</term></item>
    /// <item><term>HistoryQosPolicy.Depth</term><term>1</term></item>
    /// <item><term rowspan="3">DataWriterQos.ResourceLimits</term><term>ResourceLimitsQosPolicy.MaxInstances</term><term>Length.Unlimited</term></item>
    /// <item><term>ResourceLimitsQosPolicy.MaxSamples</term><term>Length.Unlimited</term></item>
    /// <item><term>ResourceLimitsQosPolicy.MaxSamplesPerInstance</term><term>Length.Unlimited</term></item>
    /// <item><term>DataWriterQos.TransportPriority</term><term>TransportPriorityQosPolicy.Value</term><term>0</term></item>
    /// <item><term>DataWriterQos.Lifespan</term><term>LifespanQosPolicy.Duration</term><term>Duration.Infinite</term></item>
    /// <item><term>DataWriterQos.Ownership</term><term>OwnershipQosPolicy.Kind</term><term>SharedOwnershipQos</term></item>
    /// <item><term>DataWriterQos.OwnershipStrength</term><term>OwnershipStrengthQosPolicy.Value</term><term>0</term></item>
    /// <item><term rowspan="3">DataWriterQos.WriterDataLifecycle</term><term>WriterDataLifecycleQosPolicy.AutoDisposeUnregisteredInstances</term><term>true</term></item>
    /// <item><term>WriterDataLifecycleQosPolicy.AutopurgeSuspendedSamplesDelay</term><term>Duration.Infinite</term></item>
    /// <item><term>WriterDataLifecycleQosPolicy.AutounregisterInstanceDelay</term><term>Duration.Infinite</term></item>
    /// </list>
    ///
    /// @see @ref DCPS_QoS
    /// </para>
    /// </remarks>
    public struct DataWriterQos
    {
        /// <summary>
        /// Specifies whether the data should be stored for late joining readers.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_Durability "Durability QoS"
        /// </remarks>
        public DurabilityQosPolicy Durability;
        /// <summary>
        /// Specifies the behaviour of the “transient/persistent service” of the Data Distribution System
        /// regarding Transient and Persistent Topic instances.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_DurabilityService "DurabilityService QoS"
        /// </remarks>
        public DeadlineQosPolicy Deadline;
        /// <summary>
        /// Used by the Data Distribution Service for optimization.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_LatencyBudget "LatencyBudget QoS"
        /// </remarks>
        public LatencyBudgetQosPolicy LatencyBudget;
        /// <summary>
        /// Specifies the way the liveliness of the IDataWriter is asserted to the Data Distribution Service.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_Liveliness "Liveliness QoS"
        /// </remarks>
        public LivelinessQosPolicy Liveliness;
        /// <summary>
        /// Specifies the reliability of the data distribution.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_Reliability "Reliability QoS"
        /// </remarks>
        public ReliabilityQosPolicy Reliability;
        /// <summary>
        /// Specifies the order in which the IDataReader timely orders the data.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_DestinationOrder "DestinationOrder QoS"
        /// </remarks>
        public DestinationOrderQosPolicy DestinationOrder;
        /// <summary>
        /// Specifies how samples should be stored.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_History "History QoS"
        /// </remarks>
        public HistoryQosPolicy History;
        /// <summary>
        /// Specifies the maximum amount of resources to be used.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_ResourceLimits "ResourceLimits QoS"
        /// </remarks>
        public ResourceLimitsQosPolicy ResourceLimits;
        /// <summary>
        /// Specifies a priority hint for the underlying transport layer.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_TransportPriority "TransportPriority QoS"
        /// </remarks>
        public TransportPriorityQosPolicy TransportPriority;
        /// <summary>
        /// Specifies the maximum duration of validity of the data written by a IDataWriter.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_Lifespan "Lifespan QoS"
        /// </remarks>
        public LifespanQosPolicy Lifespan;
        /// <summary>
        /// Used to attach additional information to the IDataWriter.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_TopicData "Topic Data QoS"
        public UserDataQosPolicy UserData;
        /// <summary>
        /// Specifies whether a IDataWriter exclusively owns an instance.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_Ownership "Ownership QoS"
        /// </remarks>
        public OwnershipQosPolicy Ownership;
        /// <summary>
        /// Specifies the strength to determine the ownership.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_OwnershipStrength "Ownership Strength QoS"
        /// </remarks>
        public OwnershipStrengthQosPolicy OwnershipStrength;
        /// <summary>
        /// Specifies whether unregistered instances are disposed of automatically or not.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_WriterDataLifecycle "Writer Data Lifecycle QoS"
        /// </remarks>
        public WriterDataLifecycleQosPolicy WriterDataLifecycle;
    }



    /// <summary>
    /// This struct provides the basic mechanism for an application to specify Quality of
    /// Service attributes for an IPublisher.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A QosPolicy can be set when the IPublisher is created with the
    /// IDomainParticipant.CreatePublisher operation (or modified with the SetQos operation).
    /// Both operations take the PublisherQos object as a parameter. There may be cases
    /// where several policies are in conflict. Consistency checking is performed each time
    /// the policies are modified when they are being created and, in case they are already
    /// enabled, via the SetQos operation.
    /// </para>
    /// <para>
    /// Some QosPolicy have “immutable” semantics meaning that they can only be
    /// specified either at IPublisher creation time or prior to calling the enable operation
    /// on the IPublisher.
    /// </para>
    /// <para>
    /// The initial value of the default PublisherQos in the IDomainParticipant are
    /// given in the following table:
    /// <list type="table">
    /// <listheader>
    /// <term>QosPolicy</term>
    /// <term>Attribute</term>
    /// <term><Default Value</term>
    /// </listheader>
    /// <item><term>PublisherQos.EntityFactory</term><term>EntityFactoryQosPolicy.AutoEnableCreatedEntities</term><term>true</term></item>
    /// <item><term>PublisherQos.Partition</term><term>PartitionQosPolicy.Name</term><term>null</term></item>
    /// <item><term>PublisherQos.GroupData</term><term>GroupDataQosPolicy.Value</term><term>empty</term></item>
    /// <item><term rowspan="3">PublisherQos.Presentation</term><term>PresentationQosPolicy.AccessScope</term><term>InstancePresentationQos</term></item>
    /// <item><term>PresentationQosPolicy.CoherentAccess</term><term>false</term></item>
    /// <item><term>PresentationQosPolicy.OrderedAccess</term><term>false</term></item>
    /// </list>
    ///
    /// @see @ref DCPS_QoS
    /// </para>
    /// </remarks>
    public struct PublisherQos
    {
        /// <summary>
        /// Specifies the dependency of changes to data-instances.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_Presentation "Presentation QoS"
        /// </remarks>
        public PresentationQosPolicy Presentation;
        /// <summary>
        /// Specifies the partitions in which the IPublisher is active.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_Partition "Partition QoS"
        /// </remarks>
        public PartitionQosPolicy Partition;
        /// <summary>
        /// Used to attach additional information to the IPublisher.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_GroupData "Group Data QoS"
        /// </remarks>
        public GroupDataQosPolicy GroupData;
        /// <summary>
        /// Specifies whether a just created IDataWriter should be enabled
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_EntityFactory "Entity Factory QoS"
        /// </remarks>
        public EntityFactoryQosPolicy EntityFactory;
    }

    /// <summary>
    /// This struct provides the basic mechanism for an application to specify Quality of
    /// Service attributes for an IDataReader.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A QosPolicy can be set when the IDataReader is created with the ISubscriber.CreateDataReader
    /// operation (or modified with the SetQos operation). Both operations take the
    /// DataReaderQos object as a parameter. There may be cases where several policies are in
    /// conflict. Consistency checking is performed each time the policies are modified
    /// when they are being created and, in case they are already enabled, via the SetQos
    /// operation.
    /// </para><para>
    /// Some QosPolicy have “immutable” semantics meaning that they can only be
    /// specified either at IDataReader creation time or prior to calling the Enable operation on
    /// the IDataReader.
    /// </para><para>
    /// The initial value of the default DataReaderQos in the ISubscriber are given in
    /// the following table:
    /// <list type="table">
    /// <listheader><term>QosPolicy</term><term>Attribute</term><term>Default Value</term></listheader>
    /// <item><term>DataReaderQos.UserData</term><term>UserDataQosPolicy.Value</term><term>empty</term></item>
    /// <item><term>DataReaderQos.Durability</term><term>DurabilityQosPolicy.Kind</term><term>Volatile</term></item>
    /// <item><term>DataReaderQos.Deadline</term><term>DeadlineQosPolicy.Period</term><term>Duration.Infinite</term></item>
    /// <item><term>DataReaderQos.LatencyBudget</term><term>LatencyBudgetQosPolicy.Duration</term><term>Duration.Zero</term></item>
    /// <item><term rowspan="2">DataReaderQos.Liveliness</term><term>LivelinessQosPolicy.Kind</term><term>AutomaticLivelinessQos</term></item>
    /// <item><term>LivelinessQosPolicy.LeaseDuration</term><term>Duration.Infinite</term></item>
    /// <item><term rowspan="3">DataReaderQos.Reliability</term><term>ReliabilityQosPolicy.Kind</term><term>BestEffortReliabilityQos</term></item>
    /// <item><term>ReliabilityQosPolicy.MaxBlockingTime</term><term>100 ms</term></item>
    /// <item><term>ReliabilityQosPolicy.synchronous</term><term>false</term></item>
    /// <item><term>DataReaderQos.DestinationOrder</term><term>DestinationOrderQosPolicy.Kind</term><term>ByReceptionTimestampDestinationOrderQos</term></item>
    /// <item><term rowspan="2">DataWriterQos.History</term><term>HistoryQosPolicy.Kind</term><term>KeepLastHistoryQos</term></item>
    /// <item><term>HistoryQosPolicy.Depth</term><term>1</term></item>
    /// <item><term rowspan="3">DataReaderQos.ResourceLimits</term><term>ResourceLimitsQosPolicy.MaxInstances</term><term>Length.Unlimited</term></item>
    /// <item><term>ResourceLimitsQosPolicy.MaxSamples</term><term>Length.Unlimited</term></item>
    /// <item><term>ResourceLimitsQosPolicy.MaxSamplesPerInstance</term><term>Length.Unlimited</term></item>
    /// <item><term>DataReaderQos.ReaderLifespan</term><term>ReaderLifespanQosPolicy.Duration</term><term>Duration.Infinite</term></item>
    /// <item><term>DataReaderQos.Ownership</term><term>OwnershipQosPolicy.Kind</term><term>SharedOwnershipQos</term></item>
    /// <item><term>DataReaderQos.TimeBasedFilter</term><term>TimeBasedFilterQosPolicy.MinimumSeparation</term><term>0</term></item>
    /// <item><term rowspan="5">DataReaderQos.ReaderDataLifecycle</term><term>ReaderDataLifecycleQosPolicy.AutoPurgeDisposedSamplesDelay</term><term>Duration.Infinite</term></item>
    /// <item><term>ReaderDataLifecycleQosPolicy.AutoPurgeNoWriterSamplesDelay</term><term>Duration.Infinite</term></item>
    /// <item><term>ReaderDataLifecycleQosPolicy.EnableInvalidSamples</term><term>true</term></item>
    /// <item><term>ReaderDataLifecycleQosPolicy.InvalidSampleVisibility.Kind</term><term>MinimumInvalidSamples</term></item>
    /// <item><term>ReaderDataLifecycleQosPolicy.AutopurgeDisposeAll</term><term>false</term></item>
    /// <item><term rowspan="2">DataReaderQos.SubscriptionKeys</term><term>SubscriptionKeyQosPolicy.KeyList</term><term>empty</term></item>
    /// <item><term>SubscriptionKeyQosPolicy.UseKeyList</term><term>false</term></item>
    /// <item><term rowspan="2">DataReaderQos.ReaderLifespan</term><term>ReaderLifespanQosPolicy.Duration</term><term>Duration.Infinite</term></item>
    /// <item><term>ReaderLifespanQosPolicy.UseLifespan</term><term>false</term></item>
    /// <item><term rowspan="2">DataReaderQos.Share</term><term>ShareQosPolicy.Enable</term><term>false</term></item>
    /// <item><term>ShareQosPolicy.Name</term><term>null</term></item>
    /// </list>
    ///
    /// @see @ref DCPS_QoS
    /// </para>
    /// </remarks>
    public struct DataReaderQos
    {
        /// <summary>
        /// Specifies whether the data should be stored for late joining readers.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_Durability "Durability QoS"
        /// </remarks>
        public DurabilityQosPolicy Durability;
        /// <summary>
        /// Specifies the period within which a new sample is expected or written.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_Deadline "Deadline QoS"
        /// </remarks>
        public DeadlineQosPolicy Deadline;
        /// <summary>
        /// Used by the Data Distribution Service for optimization.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_LatencyBudget "LatencyBudget QoS"
        /// </remarks>
        public LatencyBudgetQosPolicy LatencyBudget;
        /// <summary>
        /// Specifies the way the liveliness of the IDataReader is asserted to the Data Distribution Service.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_Liveliness "Liveliness QoS"
        /// </remarks>
        public LivelinessQosPolicy Liveliness;
        /// <summary>
        /// Specifies the reliability of the data distribution.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_Reliability "Reliability QoS"
        /// </remarks>
        public ReliabilityQosPolicy Reliability;
        /// <summary>
        /// Specifies the order in which the IDataReader timely orders the data.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_DestinationOrder "DestinationOrder QoS"
        /// </remarks>
        public DestinationOrderQosPolicy DestinationOrder;
         /// <summary>
        /// Specifies how samples should be stored.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_History "History QoS"
        /// </remarks>
        public HistoryQosPolicy History;
        /// <summary>
        /// Specifies the maximum amount of resources to be used.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_ResourceLimits "ResourceLimits QoS"
        /// </remarks>
        public ResourceLimitsQosPolicy ResourceLimits;
        /// <summary>
        /// Used to attach additional information to the IDataReadder.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_UserData "User Data QoS"
        /// </remarks>
        public UserDataQosPolicy UserData;
        /// <summary>
        /// Specifies whether a IDataWriter exclusively owns an instance.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_Ownership "Ownership QoS"
        /// </remarks>
        public OwnershipQosPolicy Ownership;
        /// <summary>
        /// Specifies the maximum data rate at which the IDataReader will receive changes.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_TimeBasedFilter "Time Based Filter QoS"
        /// </remarks>
        public TimeBasedFilterQosPolicy TimeBasedFilter;
        /// <summary>
        /// Specifies the lifecycle of the data instances and samples.
        /// </summary>
        /// <remarks>
        /// This QosPolicy determines whether instance state changes (either
        /// NOT_ALIVE_NO_WRITERS_INSTANCE_STATE or NOT_ALIVE_DISPOSED_INSTANCE_STATE)
        /// are presented to the user when no corresponding samples are available to
        /// communicate them. Also it determines how long an instance state change
        /// remains available to a user that does not explicitly consume them.
        ///
        /// For detailed information see @ref DCPS_QoS_ReaderDataLifecycle "Reader Data Lifecycle QoS"
        /// </remarks>
        public ReaderDataLifecycleQosPolicy ReaderDataLifecycle;
        /// <summary>
        /// Specifies that the IDataReader should order the data with an alternative key
        /// </summary>
        /// <remarks>
        /// @note Proprietary Policy
        ///
        /// The SubscriptionKeys QosPolicy allows to specify an alternative key to be
        /// used by the IDataReader to order the received data instances. When enabled
        /// the provided subscription keys are used instead of the keys associated with
        /// the ITopic.
        /// </remarks>
        public SubscriptionKeyQosPolicy SubscriptionKeys;
        /// <summary>
        /// Specifies the maximum duration of validity of the data in the IDataReader.
        /// </summary>
        /// <remarks>
        /// @note Proprietary Policy
        /// </remarks>
        public ReaderLifespanQosPolicy ReaderLifespan;
        /// <summary>
        /// Specifies if this IDataReader is shared DataReader
        /// </summary>
        /// <remarks>
        /// @note Proprietary Policy
        ///
        /// The Snare QoSPolicy is used to have several API IDataReader entities to refer to
        /// the same underlying DataReader instance. When enables the Name attribute is used
        /// to combine the DataReaders which have the same Name.
        /// </remarks>
        public ShareQosPolicy Share;
    }


    /// <summary>
    /// This struct provides the basic mechanism for an application to specify Quality of
    /// Service attributes for an ISubscriber.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A QosPolicy can be set when the ISubscriber is created with the
    /// IDomainParticipant.CreateSubscriber operation (or modified with the SetQos operation).
    /// Both operations take the SubscriberQos object as a parameter. There may be cases
    /// where several policies are in conflict. Consistency checking is performed each time
    /// the policies are modified when they are being created and, in case they are already
    /// enabled, via the SetQos operation.
    /// </para>
    /// <para>
    /// Some QosPolicy have “immutable” semantics meaning that they can only be
    /// specified either at ISubscriber creation time or prior to calling the Enable operation
    /// on the ISubscriber.
    /// </para>
    /// <para>
    /// The initial value of the default SubscriberQos in the IDomainParticipant are
    /// given in the following table:
    /// <list type="table">
    /// <listheader>
    /// <term>QosPolicy</term>
    /// <term>Attribute</term>
    /// <term><Default Value</term>
    /// </listheader>
    /// <item><term>SubscriberQos.EntityFactory</term><term>EntityFactoryQosPolicy.AutoEnableCreatedEntities</term><term>true</term></item>
    /// <item><term>SubscriberQos.Partition</term><term>PartitionQosPolicy.Name</term><term>null</term></item>
    /// <item><term>SubscriberQos.GroupData</term><term>GroupDataQosPolicy.Value</term><term>empty</term></item>
    /// <item><term rowspan="3">SubscriberQos.Presentation</term><term>PresentationQosPolicy.AccessScope</term><term>InstancePresentationQos</term></item>
    /// <item><term>PresentationQosPolicy.CoherentAccess</term><term>false</term></item>
    /// <item><term>PresentationQosPolicy.OrderedAccess</term><term>false</term></item>
    /// <item><term rowspan="2">SubscriberQos.Share</term><term>ShareQosPolicy.Enable</term><term>false</term></item>
    /// <item><term>ShareQosPolicy.Name</term><term>null</term></item>
    /// </list>
    ///
    /// @see @ref DCPS_QoS
    /// </para>
    /// </remarks>
    public struct SubscriberQos
    {
        /// <summary>
        /// Specifies the dependency of changes to data-instances.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_Presentation "Presentation QoS"
        /// </remarks>
        public PresentationQosPolicy Presentation;
        /// <summary>
        /// Specifies the partitions in which the ISubscriber is active.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_Partition "Partition QoS"
        /// </remarks>
        public PartitionQosPolicy Partition;
        /// <summary>
        /// Used to attach additional information to the ISubscriber.
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_GroupData "Group Data QoS"
        /// </remarks>
        public GroupDataQosPolicy GroupData;
        /// <summary>
        /// Specifies whether a just created IDataReader should be enabled
        /// </summary>
        /// <remarks>
        /// For detailed information see @ref DCPS_QoS_EntityFactory "Entity Factory QoS"
        /// </remarks>
        public EntityFactoryQosPolicy EntityFactory;
        /// <summary>
        /// Specifies if this ISubsscriber is shared subscriber
        /// </summary>
        /// <remarks>
        /// @note Proprietary Policy
        ///
        /// The Snare QoSPolicy is used to have several API ISubscriber entities to refer to
        /// the same underlying subscriber instance. When enables the Name attribute is used
        /// to combine the subscribers which have the same Name.
        /// </remarks>
        public ShareQosPolicy Share;
    }

    // ----------------------------------------------------------------------
    // BuiltinTopicData
    // ----------------------------------------------------------------------

    /// <summary>
    /// Represents a globally unique identifier to be used as key for the
    /// builtin topics.
    /// </summary>
    public class BuiltinTopicKey
    {
       public int[] Value;
    }

    /// <summary>
    /// Class that contains information about available DomainParticipants within
    /// the system.
    /// </summary>
    /// <remarks>
    /// The DCPSParticipant topic communicates the existence of DomainParticipants
    /// by means of the ParticipantBuiltinTopicData datatype. Each
    /// ParticipantBuiltinTopicData sample in a Domain represents a DomainParticipant
    /// that participates in that Domain: a new ParticipantBuiltinTopicData instance
    /// is created when a newly-added DomainParticipant is enabled, and it is disposed
    /// when that DomainParticipant is deleted. An updated ParticipantBuiltinTopicData
    /// sample is written each time the DomainParticipant modifies its UserDataQosPolicy.
    ///
    /// <code>
    /// /* Defaults are used and possible errors are ignored. */
    ///
    /// /* Get builtin subscriber. */
    /// DDS.DomainParticipantFactory factory = DDS.DomainParticipantFactory.Instance;
    /// DDS.IDomainParticipant participant = factory.CreateParticipant(DDS.DomainId.Default);
    /// DDS.ISubscriber builtinSubscriber = participant.BuiltInSubscriber;
    ///
    /// /* Get DCPSParticipant builtin reader. */
    /// ParticipantBuiltinTopicDataDataReader builtinReader =
    ///     (ParticipantBuiltinTopicDataDataReader)builtinSubscriber.LookupDataReader("DCPSParticipant");
    ///
    /// /* The builtinReader can now be used just as a normal typed DataReader (like Space.FooDataReader)
    ///  * to get DDS.ParticipantBuiltinTopicData samples. */
    /// </code>
    /// </remarks>
    ///
    /// @see @ref DCPS_Builtin_Topics
    /// @see @ref DCPS_Builtin_Topics_ParticipantData
    ///
    public class ParticipantBuiltinTopicData
    {
        /// <summary>
        /// Globally unique identifier of the participant
        /// </summary>
        public BuiltinTopicKey Key;

        /// <summary>
        /// User-defined data attached to the participant via a QosPolicy
        /// </summary>
        public UserDataQosPolicy UserData;
    }

    /// <summary>
    /// Class that contains information about available Topics within
    /// the system.
    /// </summary>
    /// <remarks>
    /// The DCPSTopic topic communicates the existence of topics by means of the
    /// TopicBuiltinTopicData datatype. Each TopicBuiltinTopicData sample in
    /// a Domain represents a Topic in that Domain: a new TopicBuiltinTopicData
    /// instance is created when a newly-added Topic is enabled. However, the instance is
    /// not disposed when a Topic is deleted by its participant because a topic lifecycle
    /// is tied to the lifecycle of a Domain, not to the lifecycle of an individual
    /// participant. An updated TopicBuiltinTopicData sample is written each time a
    /// Topic modifies one or more of its QosPolicy values.
    ///
    /// <code>
    /// /* Defaults are used and possible errors are ignored. */
    ///
    /// /* Get builtin subscriber. */
    /// DDS.DomainParticipantFactory factory = DDS.DomainParticipantFactory.Instance;
    /// DDS.IDomainParticipant participant = factory.CreateParticipant(DDS.DomainId.Default);
    /// DDS.ISubscriber builtinSubscriber = participant.BuiltInSubscriber;
    ///
    /// /* Get DCPSTopic builtin reader. */
    /// TopicBuiltinTopicDataDataReader builtinReader =
    ///     (TopicBuiltinTopicDataDataReader)builtinSubscriber.LookupDataReader("DCPSTopic");
    ///
    /// /* The builtinReader can now be used just as a normal typed DataReader (like Space.FooDataReader)
    ///  * to get DDS.TopicBuiltinTopicData samples. */
    /// </code>
    /// </remarks>
    ///
    /// @see @ref DCPS_Builtin_Topics
    /// @see @ref DCPS_Builtin_Topics_TopicData
    ///
    public class TopicBuiltinTopicData
    {
        /// <summary>
        /// Global unique identifier of the Topic
        /// </summary>
        public BuiltinTopicKey Key;

        /// <summary>
        /// Name of the Topic
        /// </summary>
        public string Name;

        /// <summary>
        /// Type name of the Topic (i.e. the fully scoped IDL name)
        /// </summary>
        public string TypeName;

        /// <summary>
        /// QosPolicy attached to the Topic
        /// </summary>
        public DurabilityQosPolicy Durability;

        /// <summary>
        /// QosPolicy attached to the Topic
        /// </summary>
        public DurabilityServiceQosPolicy DurabilityService;

        /// <summary>
        /// QosPolicy attached to the Topic
        /// </summary>
        public DeadlineQosPolicy Deadline;

        /// <summary>
        /// QosPolicy attached to the Topic
        /// </summary>
        public LatencyBudgetQosPolicy LatencyBudget;

        /// <summary>
        /// QosPolicy attached to the Topic
        /// </summary>
        public LivelinessQosPolicy Liveliness;

        /// <summary>
        /// QosPolicy attached to the Topic
        /// </summary>
        public ReliabilityQosPolicy Reliability;

        /// <summary>
        /// QosPolicy attached to the Topic
        /// </summary>
        public TransportPriorityQosPolicy TransportPriority;

        /// <summary>
        /// QosPolicy attached to the Topic
        /// </summary>
        public LifespanQosPolicy Lifespan;

        /// <summary>
        /// QosPolicy attached to the Topic
        /// </summary>
        public DestinationOrderQosPolicy DestinationOrder;

        /// <summary>
        /// QosPolicy attached to the Topic
        /// </summary>
        public HistoryQosPolicy History;

        /// <summary>
        /// QosPolicy attached to the Topic
        /// </summary>
        public ResourceLimitsQosPolicy ResourceLimits;

        /// <summary>
        /// QosPolicy attached to the Topic
        /// </summary>
        public OwnershipQosPolicy Ownership;

        /// <summary>
        /// QosPolicy attached to the Topic
        /// </summary>
        public TopicDataQosPolicy TopicData;
    }

    /// <summary>
    /// Class that contains information about available DataWriters within
    /// the system.
    /// </summary>
    /// <remarks>
    /// The DCPSPublication topic communicates the existence of datawriters by means
    /// of the PublicationBuiltinTopicData datatype. Each PublicationBuiltinTopicData
    /// sample in a Domain represents a datawriter in that Domain: a new
    /// PublicationBuiltinTopicData instance is created when a newly-added DataWriter
    /// is enabled, and it is disposed when that DataWriter is deleted. An updated
    /// PublicationBuiltinTopicData sample is written each time the DataWriter (or
    /// the Publisher to which it belongs) modifies a QosPolicy that applies to the
    /// entities connected to it. Also will it be updated when the writer looses or
    /// regains its liveliness.
    ///
    /// <code>
    /// /* Defaults are used and possible errors are ignored. */
    ///
    /// /* Get builtin subscriber. */
    /// DDS.DomainParticipantFactory factory = DDS.DomainParticipantFactory.Instance;
    /// DDS.IDomainParticipant participant = factory.CreateParticipant(DDS.DomainId.Default);
    /// DDS.ISubscriber builtinSubscriber = participant.BuiltInSubscriber;
    ///
    /// /* Get DCPSPublication builtin reader. */
    /// PublicationBuiltinTopicDataDataReader builtinReader =
    ///     (PublicationBuiltinTopicDataDataReader)builtinSubscriber.LookupDataReader("DCPSPublication");
    ///
    /// /* The builtinReader can now be used just as a normal typed DataReader (like Space.FooDataReader)
    ///  * to get DDS.PublicationBuiltinTopicData samples. */
    /// </code>
    /// </remarks>
    ///
    /// @see @ref DCPS_Builtin_Topics
    /// @see @ref DCPS_Builtin_Topics_PublicationData
    ///
    public class PublicationBuiltinTopicData
    {
        /// <summary>
        /// Global unique identifier of the DataWriter
        /// </summary>
        public BuiltinTopicKey Key;

        /// <summary>
        /// Global unique identifier of the Participant to which the DataWriter belongs
        /// </summary>
        public BuiltinTopicKey ParticipantKey;

        /// <summary>
        /// Name of the Topic used by the DataWriter
        /// </summary>
        public string TopicName;

        /// <summary>
        /// Type name of the Topic used by the DataWriter
        /// </summary>
        public string TypeName;

        /// <summary>
        /// QosPolicy attached to the DataWriter
        /// </summary>
        public DurabilityQosPolicy Durability;

        /// <summary>
        /// QosPolicy attached to the DataWriter
        /// </summary>
        public DeadlineQosPolicy Deadline;

        /// <summary>
        /// QosPolicy attached to the DataWriter
        /// </summary>
        public LatencyBudgetQosPolicy LatencyBudget;

        /// <summary>
        /// QosPolicy attached to the DataWriter
        /// </summary>
        public LivelinessQosPolicy Liveliness;

        /// <summary>
        /// QosPolicy attached to the DataWriter
        /// </summary>
        public ReliabilityQosPolicy Reliability;

        /// <summary>
        /// QosPolicy attached to the DataWriter
        /// </summary>
        public LifespanQosPolicy Lifespan;

        /// <summary>
        /// QosPolicy attached to the DataWriter
        /// </summary>
        public UserDataQosPolicy UserData;

        /// <summary>
        /// QosPolicy attached to the DataWriter
        /// </summary>
        public OwnershipQosPolicy Ownership;

        /// <summary>
        /// QosPolicy attached to the DataWriter
        /// </summary>
        public OwnershipStrengthQosPolicy OwnershipStrength;

        /// <summary>
        /// QosPolicy attached to the Publisher to which the DataWriter belongs
        /// </summary>
        public PresentationQosPolicy Presentation;

        /// <summary>
        /// QosPolicy attached to the Publisher to which the DataWriter belongs
        /// </summary>
        public PartitionQosPolicy Partition;

        /// <summary>
        /// QosPolicy attached to the Publisher to which the DataWriter belongs
        /// </summary>
        public TopicDataQosPolicy TopicData;

        /// <summary>
        /// QosPolicy attached to the Publisher to which the DataWriter belongs
        /// </summary>
        public GroupDataQosPolicy GroupData;
    }

    /// <summary>
    /// Class that contains information about available DataReaders within
    /// the system.
    /// </summary>
    /// <remarks>
    /// The DCPSSubscription topic communicates the existence of datareaders by
    /// means of the SubscriptionBuiltinTopicData datatype. Each
    /// SubscriptionBuiltinTopicData sample in a Domain represents a datareader
    /// in that Domain: a new SubscriptionBuiltinTopicData instance is created
    /// when a newly-added DataReader is enabled, and it is disposed when that
    /// DataReader is deleted. An updated SubscriptionBuiltinTopicData sample is
    /// written each time the DataReader (or the Subscriber to which it belongs)
    /// modifies a QosPolicy that applies to the entities connected to it.
    ///
    /// <code>
    /// /* Defaults are used and possible errors are ignored. */
    ///
    /// /* Get builtin subscriber. */
    /// DDS.DomainParticipantFactory factory = DDS.DomainParticipantFactory.Instance;
    /// DDS.IDomainParticipant participant = factory.CreateParticipant(DDS.DomainId.Default);
    /// DDS.ISubscriber builtinSubscriber = participant.BuiltInSubscriber;
    ///
    /// /* Get DCPSSubscription builtin reader. */
    /// SubscriptionBuiltinTopicDataDataReader builtinReader =
    ///     (SubscriptionBuiltinTopicDataDataReader)builtinSubscriber.LookupDataReader("DCPSSubscription");
    ///
    /// /* The builtinReader can now be used just as a normal typed DataReader (like Space.FooDataReader)
    ///  * to get DDS.SubscriptionBuiltinTopicData samples. */
    /// </code>
    /// </remarks>
    ///
    /// @see @ref DCPS_Builtin_Topics
    /// @see @ref DCPS_Builtin_Topics_PublicationData
    ///
    public class SubscriptionBuiltinTopicData
    {
        /// <summary>
        /// Global unique identifier of the DataReader
        /// </summary>
        public BuiltinTopicKey Key;

        /// <summary>
        /// Global unique identifier of the Participant to which the DataReader belongs
        /// </summary>
        public BuiltinTopicKey ParticipantKey;

        /// <summary>
        /// Name of the Topic used by the DataReader
        /// </summary>
        public string TopicName;

        /// <summary>
        /// Type name of the Topic used by the DataReader
        /// </summary>
        public string TypeName;

        /// <summary>
        /// QosPolicy attached to the DataReader
        /// </summary>
        public DurabilityQosPolicy Durability;

        /// <summary>
        /// QosPolicy attached to the DataReader
        /// </summary>
        public DeadlineQosPolicy Deadline;

        /// <summary>
        /// QosPolicy attached to the DataReader
        /// </summary>
        public LatencyBudgetQosPolicy LatencyBudget;

        /// <summary>
        /// QosPolicy attached to the DataReader
        /// </summary>
        public LivelinessQosPolicy Liveliness;

        /// <summary>
        /// QosPolicy attached to the DataReader
        /// </summary>
        public ReliabilityQosPolicy Reliability;

        /// <summary>
        /// QosPolicy attached to the DataReader
        /// </summary>
        public OwnershipQosPolicy Ownership;

        /// <summary>
        /// QosPolicy attached to the DataReader
        /// </summary>
        public DestinationOrderQosPolicy DestinationOrder;

        /// <summary>
        /// QosPolicy attached to the DataReader
        /// </summary>
        public UserDataQosPolicy UserData;

        /// <summary>
        /// QosPolicy attached to the DataReader
        /// </summary>
        public TimeBasedFilterQosPolicy TimeBasedFilter;

        /// <summary>
        /// QosPolicy attached to the Subscriber to which the DataReader belongs
        /// </summary>
        public PresentationQosPolicy Presentation;

        /// <summary>
        /// QosPolicy attached to the Subscriber to which the DataReader belongs
        /// </summary>
        public PartitionQosPolicy Partition;

        /// <summary>
        /// QosPolicy attached to the Subscriber to which the DataReader belongs
        /// </summary>
        public TopicDataQosPolicy TopicData;

        /// <summary>
        /// QosPolicy attached to the Subscriber to which the DataReader belongs
        /// </summary>
        public GroupDataQosPolicy GroupData;
    }

    /// <summary>
    /// The proprietary builtin CMParticipantBuiltinTopicData topic.
    /// </summary>
    /// <remarks>
    /// This topic is normally only used by OpenSplice tools.
    ///
    /// @note
    /// Users are discouraged from doing anything with this topic, so as not to interfere
    /// with internal mechanisms that rely on them. The structure of these topics may
    /// change without notification.
    /// </remarks>
    public class CMParticipantBuiltinTopicData
    {
        public BuiltinTopicKey key;
        public ProductDataQosPolicy product;
    }

    /// <summary>
    /// The proprietary builtin CMPublisherBuiltinTopicData topic.
    /// </summary>
    /// <remarks>
    /// This topic is normally only used by OpenSplice tools.
    ///
    /// @note
    /// Users are discouraged from doing anything with this topic, so as not to interfere
    /// with internal mechanisms that rely on them. The structure of these topics may
    /// change without notification.
    /// </remarks>
    public class CMPublisherBuiltinTopicData
    {
        public BuiltinTopicKey key;
        public ProductDataQosPolicy product;
        public BuiltinTopicKey participant_key;
        public string name;
        public EntityFactoryQosPolicy entity_factory;
        public PartitionQosPolicy partition;
    }

    /// <summary>
    /// The proprietary builtin CMSubscriberBuiltinTopicData topic.
    /// </summary>
    /// <remarks>
    /// This topic is normally only used by OpenSplice tools.
    ///
    /// @note
    /// Users are discouraged from doing anything with this topic, so as not to interfere
    /// with internal mechanisms that rely on them. The structure of these topics may
    /// change without notification.
    /// </remarks>
    public class CMSubscriberBuiltinTopicData
    {
        public BuiltinTopicKey Key;
        public ProductDataQosPolicy Product;
        public BuiltinTopicKey ParticipantKey;
        public string Name;
        public EntityFactoryQosPolicy EntityFactory;
        public ShareQosPolicy Share;
        public PartitionQosPolicy Partition;
    }

    /// <summary>
    /// The proprietary builtin CMDataWriterBuiltinTopicData topic.
    /// </summary>
    /// <remarks>
    /// This topic is normally only used by OpenSplice tools.
    ///
    /// @note
    /// Users are discouraged from doing anything with this topic, so as not to interfere
    /// with internal mechanisms that rely on them. The structure of these topics may
    /// change without notification.
    /// </remarks>
    public class CMDataWriterBuiltinTopicData
    {
        public BuiltinTopicKey Key;
        public ProductDataQosPolicy Product;
        public BuiltinTopicKey PublisherKey;
        public string Name;
        public HistoryQosPolicy History;
        public ResourceLimitsQosPolicy ResourceLimits;
        public WriterDataLifecycleQosPolicy WriterDataLifecycle;
    }

    /// <summary>
    /// The proprietary builtin CMDataReaderBuiltinTopicData topic.
    /// </summary>
    /// <remarks>
    /// This topic is normally only used by OpenSplice tools.
    ///
    /// @note
    /// Users are discouraged from doing anything with this topic, so as not to interfere
    /// with internal mechanisms that rely on them. The structure of these topics may
    /// change without notification.
    /// </remarks>
    public class CMDataReaderBuiltinTopicData
    {
        public BuiltinTopicKey Key;
        public ProductDataQosPolicy Product;
        public BuiltinTopicKey SubscriberKey;
        public string Name;
        public HistoryQosPolicy History;
        public ResourceLimitsQosPolicy ResourceLimits;
        public ReaderDataLifecycleQosPolicy ReaderDataLifecycle;
        public UserKeyQosPolicy SubscriptionKeys;
        public ReaderLifespanQosPolicy ReaderLifespan;
        public ShareQosPolicy share;
    }

#endif // DOXYGEN_FOR_CS


    /// <summary>
    /// The class SampleInfo represents the additional information that accompanies the
    /// data in each sample that is read or taken.
    /// </summary>
    /// <remarks>
    /// <para><b><i>Generations</i></b></para>
    /// <para>
    /// A generation is defined as: ‘the number of times an instance has become alive (with
    /// instance_state==ALIVE_INSTANCE_STATE) at the time the sample was
    /// received’. Note that the generation counters are initialized to zero when a
    /// DataReader first detects a never-seen-before instance.
    /// Two types of generations are distinguished: DisposedGenerationCount and
    /// NoWritersGenerationCount.
    /// After a DataWriter disposes an instance, the disposed_generation_count
    /// for all DataReaders that already knew that instance will be incremented the next
    /// time the instance is written again.
    /// If the DataReader detects that there are no live DataWriter entities, the
    /// instance_state of the sample_info will change from
    /// ALIVE_INSTANCE_STATE to NOT_ALIVE_NO_WRITERS_INSTANCE_STATE. The
    /// next time the instance is written, no_writers_generation_count will be
    /// incremented.
    /// </para>
    /// <para><b><i>Sample Information</i></b></para>
    /// <para>
    /// SampleInfo is the additional information that accompanies the data in each sample
    /// that is ‘read’ or ‘taken’. It contains the following information:
    /// <list>
    /// <item>SampleState (READ_SAMPLE_STATE or NOT_READ_SAMPLE_STATE)
    /// indicates whether or not the corresponding data sample has already been read.</item>
    /// <item>ViewState (NEW_VIEW_STATE or NOT_NEW_VIEW_STATE) indicates whether
    /// the DataReader has already seen samples of the most-current generation of the
    /// related instance.</item>
    /// <item>InstanceState (ALIVE_INSTANCE_STATE, NOT_ALIVE_DISPOSED_
    /// INSTANCE_STATE, or NOT_ALIVE_NO_WRITERS_INSTANCE_STATE) indicates
    /// whether the instance is alive, has no writers or if it has been disposed of:
    /// <list>
    /// <item>ALIVE_INSTANCE_STATE if this instance is currently in existence.</item>
    /// <item>NOT_ALIVE_DISPOSED_INSTANCE_STATE if this instance was disposed of by a DataWriter.</item>
    /// <item>NOT_ALIVE_NO_WRITERS_INSTANCE_STATE none of the DataWriter objects currently
    /// “alive” (according to the LivelinessQosPolicy) are writing the instance.</item>
    /// </list></item>
    /// <item>SourceTimestamp indicates the time provided by the DataWriter when the sample was written.</item>
    /// <item>InstanceHandle indicates locally the corresponding instance.</item>
    /// <item>PublicationHandle is used by the DDS implementation to locally identify
    /// the corresponding source DataWriter. You can access more detailed information
    /// about this particular publication by passing its PublicationHandle to either
    /// the get_matched_publication_data operation on the DataReader or to the
    /// read_instance operation on the built-in reader for the “DCPSPublication" "topic.</item>
    /// </list>
    /// Be aware that since an Instance Handle is an opaque datatype, it does not
    /// necessarily mean that the handle obtained from the publication_handle has
    /// the same value as the one that appears in the instance_handle field of the
    /// SampleInfo when retrieving the publication info through corresponding
    /// "DCPSPublication" built-in reader. You can’t just compare two handles to
    /// determine whether they represent the same publication. If you want to know
    /// whether two handles actually do represent the same publication, use both handles
    /// to retrieve their corresponding PublicationBuiltinTopicData samples and
    /// then compare the key field of both samples.
    [StructLayoutAttribute(LayoutKind.Sequential)]
    public class SampleInfo
    {
        /// <summary>
        /// The sample_state of the Data value (i.e., if the sample has already been READ or NOT_READ by that same DataReader).
        /// </summary>
        public SampleStateKind SampleState;
        /// <summary>
        /// The view_state of the related instance (i.e., if the instance is NEW, or NOT_NEW for that DataReader).
        /// </summary>
        public ViewStateKind ViewState;
        /// <summary>
        /// The instance_state of the related instance (i.e., if the instance is ALIVE, NOT_ALIVE_DISPOSED, or NOT_ALIVE_NO_WRITERS).
        /// </summary>
        public InstanceStateKind InstanceState;
        /// <summary>
        /// The number of times the instance has become alive after it was disposed of explicitly by a DataWriter.
        /// </summary>
        public int DisposedGenerationCount;
        /// <summary>
        /// The number of times the instance has become alive after it was disposed of because there were no DataWriter objects.
        /// </summary>
        public int NoWritersGenerationCount;
        /// <summary>
        /// The number of samples related to the same instance that are found in the collection returned by a read or take operation.
        /// </summary>
        public int SampleRank;
        /// <summary>
        /// The generation difference between the time the sample was received and the time the most recent sample in the collection was received.
        /// </summary>
        public int GenerationRank;
        /// <summary>
        /// The generation difference between the time the sample was received and the time the most recent sample was received.
        /// </summary>
        public int AbsoluteGenerationRank;
        /// <summary>
        /// The timestamp provided by the DataWriter at the time the sample was produced.
        /// </summary>
        public Time SourceTimestamp;
        /// <summary>
        /// The handle that identifies locally the corresponding instance.
        /// </summary>
        public InstanceHandle InstanceHandle;
        /// <summary>
        /// The handle that identifies locally the DataWriter that modified the instance.
        /// </summary>
        /// <remarks>
        /// In fact it is an instance_handle of the built-in DCPSPublication sample that
        /// describes this DataWriter. It can be used as a parameter to the DataReader
        /// operation GetMatchedPublicationData to obtain this built-in DCPSPublication sample.
        /// </remarks>
        public InstanceHandle PublicationHandle;
        /// <summary>
        /// Indicates whether the DataSample contains any meaningful data.
        /// </summary>
        /// <remarks>
        /// If not, the sample is only used to communicate a change in the instance_state
        /// of the instance.
        /// </remarks>
        [MarshalAs(UnmanagedType.U1)]
        public bool ValidData;
        /// <summary>
        /// The timestamp provided by the DataReader when the sample was received.
        /// </summary>
        public Time ReceptionTimestamp;
    }

} // end namespace DDS
