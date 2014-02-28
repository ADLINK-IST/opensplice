#ifndef OMG_DDS_SUB_DATA_STATE_HPP_
#define OMG_DDS_SUB_DATA_STATE_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <bitset>

#include <dds/core/types.hpp>


namespace dds
{
namespace sub
{
namespace status
{
class SampleState;
class ViewState;
class InstanceState;
class DataState;
}
}
}

/**
* For each sample, the Data Distribution Service internally maintains a
* sample_state specific to each DataReader. The sample_state can either be
* READ_SAMPLE_STATE or NOT_READ_SAMPLE_STATE.
*
* READ_SAMPLE_STATE indicates that the DataReader has already accessed that
*   sample by means of read. Had the sample been accessed by take it would
*   no longer be available to the DataReader.
*
* NOT_READ_SAMPLE_STATE indicates that the DataReader has not accessed
* that sample before.
**/
class OMG_DDS_API dds::sub::status::SampleState : public std::bitset<OMG_DDS_STATE_BIT_COUNT>
{
public:
    typedef std::bitset<OMG_DDS_STATE_BIT_COUNT> MaskType;

public:
    /**
     * Contruct a SampleState with default MaskType.
     */
    SampleState();

    /**
     * Contruct a SampleState with MaskType of i.
     *
     * @param i MaskType
     */
    explicit SampleState(uint32_t i);

    /**
     * Copy constructor.
     * Contruct a SampleState with existing SampleState.
     *
     * @param src the SampleState to copy from
     */
    SampleState(const SampleState& src);

    /**
     * Construct a SampleState with existing MaskType.
     *
     * @param src the MaskType to copy from
     */
    SampleState(const MaskType& src);

public:
    /**
     * Get the READ_SAMPLE_STATE.
     *
     * @return the read SampleState
     */
    inline static const SampleState read();

    /**
     * Get the NOT_READ_SAMPLE_STATE.
     *
     * @return the not_read SampleState
     */
    inline static const SampleState not_read();

    /**
     * Get any SampleState.
     * (Either the sample has already been read or not read)
     *
     * @return any SampleState
     */
    inline static const SampleState any();

};

/**
 * For each instance (identified by the key), the Data Distribution Service internally
 * maintains a view_state relative to each DataReader. The ViewSate can
 * either be NEW_VIEW_STATE or NOT_NEW_VIEW_STATE.
 *
 * NEW_VIEW_STATE indicates that either this is the first time that the DataReader
 *   has ever accessed samples of that instance, or else that the DataReader has
 *   accessed previous samples of the instance, but the instance has since been reborn
 *   (i.e. become not-alive and then alive again).
 *
 * NOT_NEW_VIEW_STATE indicates that the DataReader has already accessed
 *   samples of the same instance and that the instance has not been reborn since.
 */
class OMG_DDS_API dds::sub::status::ViewState : public std::bitset<OMG_DDS_STATE_BIT_COUNT>
{
public:
    typedef std::bitset<OMG_DDS_STATE_BIT_COUNT> MaskType;

public:
    /**
     * Contruct a ViewState with default MaskType.
     */
    ViewState();

    /**
     * Contruct a ViewState with MaskType of i.
     *
     * @param m the MaskType
     */
    explicit ViewState(uint32_t m);

    /**
     * Copy constructor.
     * Contruct a ViewState with existing ViewState.
     *
     * @param src the ViewState to copy from
     */
    ViewState(const ViewState& src);

    /**
     * Construct a ViewState with existing MaskType.
     *
     * @param src the MaskType to copy from
     */
    ViewState(const MaskType& src);

public:
    /**
     * Get the NEW_VIEW_STATE.
     *
     * @return the new_view ViewState
     */
    inline static const ViewState new_view();

    /**
     * Get the NOT_NEW_VIEW_STATE.
     *
     * @return the not_new_view ViewState
     */
    inline static const ViewState not_new_view();

    /**
     * Get any ViewState.
     * (Either the sample has already been seen or not seen.)
     *
     * @return the any ViewState
     */
    inline static const ViewState any();

};

/**
 * For each instance the Data Distribution Service internally maintains an
 * InstanceState. The InstanceState can be:
 *
 * * ALIVE_INSTANCE_STATE, which indicates that samples have been received
 *     for the instance
 *
 *       There are live DataWriter objects writing the instance
 *       the instance has not been explicitly disposed of (or else samples have been
 *       received after it was disposed of)
 *
 * * NOT_ALIVE_DISPOSED_INSTANCE_STATE indicates the instance was disposed
 *     of by a DataWriter, either explicitly by means of the dispose operation or
 *     implicitly in case the autodispose_unregistered_instances field of the
 *     WriterDataLyfecycleQosPolicy equals TRUE when the instance gets
 *     unregistered, WriterDataLifecycleQosPolicy and no new
 *     samples for that instance have been written afterwards.
 *
 * * NOT_ALIVE_NO_WRITERS_INSTANCE_STATE indicates the instance has been
 *     declared as not-alive by the DataReader because it detected that there are no live
 *     DataWriter objects writing that instance.
 */
class OMG_DDS_API dds::sub::status::InstanceState : public std::bitset<OMG_DDS_STATE_BIT_COUNT>
{
public:
    typedef std::bitset<OMG_DDS_STATE_BIT_COUNT> MaskType;

public:
    /**
     * Contruct an InstanceState with default MaskType.
     */
    InstanceState();

    /**
     * Contruct an InstanceState with MaskType of i.
     *
     * @param m the MaskType
     */
    explicit InstanceState(uint32_t m);

    /**
     * Copy constructor.
     * Contruct an InstanceState with existing InstanceState.
     *
     * @param src the InstanceState to copy from
     */
    InstanceState(const InstanceState& src);

    /**
     * Construct an InstanceState with existing MaskType.
     *
     * @param src the MaskType to copy from
     */
    InstanceState(const MaskType& src);

public:
    /**
     * Get ALIVE_INSTANCE_STATE.
     *
     * @return the alive InstanceState
     */
    inline static const InstanceState alive();

    /**
     * Get NOT_ALIVE_DISPOSED_INSTANCE_STATE.
     *
     * @return the not_alive_disposed InstanceState
     */
    inline static const InstanceState not_alive_disposed();

    /**
     * Get NOT_ALIVE_NO_WRITERS_INSTANCE_STATE.
     *
     * @return the not_alive_no_writers InstanceState
     */
    inline static const InstanceState not_alive_no_writers();

    /**
     * Get not_alive mask -
     *    NOT_ALIVE_DISPOSED_INSTANCE_STATE | NOT_ALIVE_NO_WRITERS_INSTANCE_STATE
     *
     * @return the not_alive_mask InstanceState
     */
    inline static const InstanceState not_alive_mask();

    /**
     * Get any InstanceState.
     * (This Instance is either in existence or not in existence.)
     *
     * @return the any InstanceState
     */
    inline static const InstanceState any();

};

class OMG_DDS_API dds::sub::status::DataState
{
public:
    /**
     * Contruct a DataState with -
     *  * SampleState::any
     *  * ViewState::any
     *  * InstanceState::any
     */
    DataState()
        : ss_(dds::sub::status::SampleState::any()),
          vs_(dds::sub::status::ViewState::any()),
          is_(dds::sub::status::InstanceState::any())
    { }

    /**
     * Contruct a DataState with a SampleState.
     *
     * @param ss the SampleState to construct DataState from
     */
    /* implicit */ DataState(const dds::sub::status::SampleState& ss)
        : ss_(ss),
          vs_(dds::sub::status::ViewState::any()),
          is_(dds::sub::status::InstanceState::any())
    { }

    /**
     * Contruct a DataState with a ViewState.
     *
     * @param vs the ViewState to construct DataState from
     */
    /* implicit */ DataState(const dds::sub::status::ViewState& vs)
        : ss_(dds::sub::status::SampleState::any()),
          vs_(vs),
          is_(dds::sub::status::InstanceState::any())
    { }

    /**
     * Contruct a DataState with a InstanceState.
     *
     * @param is InstanceState to construct DataState from
     */
    /* implicit */ DataState(const dds::sub::status::InstanceState& is)
        : ss_(dds::sub::status::SampleState::any()),
          vs_(dds::sub::status::ViewState::any()),
          is_(is)
    { }

    /**
     * Contruct a InstanceState with -
     *  * SampleState
     *  * ViewState
     *  * InstanceState
     *
     * @param ss SampleState
     * @param vs ViewState
     * @param is InstanceState
     */
    DataState(const dds::sub::status::SampleState& ss,
              const dds::sub::status::ViewState& vs,
              const dds::sub::status::InstanceState& is)
        : ss_(ss), vs_(vs), is_(is)
    { }

    /**
     * Set SampleState.
     *
     * @param ss SampleState
     */
    DataState& operator << (const dds::sub::status::SampleState& ss)
    {
        ss_ = ss;
        return *this;
    }

    /**
     * Set InstanceState.
     *
     * @param is InstanceState
     */
    DataState& operator << (const dds::sub::status::InstanceState& is)
    {
        is_ = is;
        return *this;
    }

    /**
     * Set ViewState.
     *
     * @param vs ViewState
     */
    DataState& operator << (const dds::sub::status::ViewState& vs)
    {
        vs_ = vs;
        return *this;
    }

    /**
     * Get SampleState.
     *
     * @param ss SampleState
     * @return the DataState
     */
    const DataState& operator >> (dds::sub::status::SampleState& ss) const
    {
        ss = ss_;
        return *this;
    }

    /**
     * Get InstanceState.
     *
     * @param is InstanceState
     * @return the DataState
     */
    const DataState& operator >> (dds::sub::status::InstanceState& is) const
    {
        is = is_;
        return *this;
    }

    /**
     * Get ViewState.
     *
     * @param vs ViewState
     * @return the DataState
     */
    const DataState& operator >> (dds::sub::status::ViewState& vs) const
    {
        vs = vs_;
        return *this;
    }

    /**
     * Get SampleState.
     *
     * @return the SampleState
     */
    const dds::sub::status::SampleState& sample_state() const
    {
        return ss_;
    }

    /**
     * Set SampleState.
     *
     * @param ss SampleState
     */
    void sample_state(const dds::sub::status::SampleState& ss)
    {
        *this << ss;
    }

    /**
     * Get InstanceState.
     *
     * @return the InstanceState
     */
    const dds::sub::status::InstanceState& instance_state() const
    {
        return is_;
    }

    /**
     * Set InstanceState.
     *
     * @param is InstanceState
     */
    void instance_state(const dds::sub::status::InstanceState& is)
    {
        *this << is;
    }

    /**
     * Get ViewState.
     *
     * @return the ViewState
     */
    const dds::sub::status::ViewState& view_state() const
    {
        return vs_;
    }

    /**
     * Set ViewState.
     *
     * @param vs ViewState
     */
    void view_state(const dds::sub::status::ViewState& vs)
    {
        *this << vs;
    }

    /**
     * Get any DataState -
     *  * SampleState::any
     *  * ViewState::any
     *  * InstanceState::any
     *
     * @return the any DataState
     */
    static DataState any()
    {
        return DataState(dds::sub::status::SampleState::any(),
                         dds::sub::status::ViewState::any(),
                         dds::sub::status::InstanceState::any());
    }

    /**
     * Get new_data DataState -
     *  * SampleState::not_read
     *  * ViewState::any
     *  * InstanceState::alive
     *
     * @return the new_data DataState
     */
    static DataState new_data()
    {
        return DataState(dds::sub::status::SampleState::not_read(),
                         dds::sub::status::ViewState::any(),
                         dds::sub::status::InstanceState::alive());
    }

    /**
     * Get any_data DataState -
     *  * SampleState::any
     *  * ViewState::any
     *  * InstanceState::alive
     *
     * @return the any_data DataState
     */
    static DataState any_data()
    {
        return DataState(dds::sub::status::SampleState::any(),
                         dds::sub::status::ViewState::any(),
                         dds::sub::status::InstanceState::alive());
    }

    /**
     * Get new_instance DataState -
     *  * SampleState::any
     *  * ViewState::new_view
     *  * InstanceState::alive
     *
     * @return the new_instance DataState
     */
    static DataState new_instance()
    {
        return DataState(dds::sub::status::SampleState::any(),
                         dds::sub::status::ViewState::new_view(),
                         dds::sub::status::InstanceState::alive());
    }
private:
    dds::sub::status::SampleState ss_;
    dds::sub::status::ViewState vs_;
    dds::sub::status::InstanceState is_;

};

#endif /* OMG_DDS_SUB_DATA_STATE_HPP_ */
