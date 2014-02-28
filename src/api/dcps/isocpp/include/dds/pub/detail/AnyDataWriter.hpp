/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2012 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef OSPL_DDS_PUB_DETAIL_ANYDATAWRITER_HPP_
#define OSPL_DDS_PUB_DETAIL_ANYDATAWRITER_HPP_

/**
 * @file
 */

// Implementation

#include <string>
#include <dds/pub/qos/DataWriterQos.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/pub/DataWriter.hpp>

// NOTE: This code is non-normative and is provided only as a guide
// to implementor of this specification.

namespace dds
{
namespace pub
{
namespace detail
{
class DWHolderBase;
template <typename T> class DWHolder;
}
}
}

class dds::pub::detail::DWHolderBase
{
public:
    virtual ~DWHolderBase() { }

    virtual const dds::pub::qos::DataWriterQos& qos() const = 0;

    virtual void qos(const ::dds::pub::qos::DataWriterQos& qos) = 0;

    virtual const std::string& topic_name() const = 0;

    virtual const std::string& type_name() const = 0;

    virtual  const dds::pub::Publisher& publisher() const = 0;

    virtual void wait_for_acknowledgments(const dds::core::Duration& timeout) = 0;

    virtual void close() = 0;

    virtual void retain(bool b) = 0;

    virtual DDS::DataWriter_ptr get_dds_datawriter() = 0;
};

template <typename T>
class dds::pub::detail::DWHolder : public DWHolderBase
{
public:
    DWHolder(const dds::pub::DataWriter<T>& dw) : dw_(dw)
    {
        dw_var_ = DDS::DataWriter::_narrow(((dds::pub::DataWriter<T>)dw)->get_raw_writer());
    }
    virtual ~DWHolder() { }
public:
    virtual const ::dds::pub::qos::DataWriterQos& qos() const
    {
        return dw_.qos();
    }

    virtual void qos(const ::dds::pub::qos::DataWriterQos& qos)
    {
        dw_.qos(qos);
    }

    virtual const std::string& topic_name() const
    {
        return dw_.topic().name();
    }

    virtual const std::string& type_name() const
    {
        return dw_.topic().type_name();
    }

    virtual const ::dds::pub::Publisher& publisher() const
    {
        return dw_.publisher();
    }

    virtual void wait_for_acknowledgments(const dds::core::Duration& timeout)
    {
        dw_.wait_for_acknowledgments(timeout);
    }

    virtual void close()
    {
        dw_.close();
    }

    virtual void retain(bool b)  { }

    const dds::pub::DataWriter<T>& get() const
    {
        return dw_;
    }

    DDS::DataWriter_ptr get_dds_datawriter()
    {
        return dw_var_.in();
    }

private:
    dds::pub::DataWriter<T> dw_;
    DDS::DataWriter_var dw_var_;
};


// End of implementation

#endif /* OSPL_DDS_PUB_DETAIL_ANYDATAWRITER_HPP_ */
