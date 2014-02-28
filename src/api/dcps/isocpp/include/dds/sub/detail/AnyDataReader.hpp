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
#ifndef OSPL_DDS_SUB_DETAIL_ANYDATAREADER_HPP_
#define OSPL_DDS_SUB_DETAIL_ANYDATAREADER_HPP_

/**
 * @file
 */

// Implementation

#include <dds/sub/DataReader.hpp>
#include <dds/sub/qos/DataReaderQos.hpp>

namespace dds
{
namespace sub
{
namespace detail
{
class DRHolderBase
{
public:
    virtual ~DRHolderBase() { }
    virtual const ::dds::sub::qos::DataReaderQos qos() const = 0;

    virtual void qos(const dds::sub::qos::DataReaderQos& qos) = 0;

    virtual const std::string topic_name() const = 0;

    virtual const std::string type_name() const = 0;

    virtual ::dds::sub::Subscriber parent() const = 0;

    virtual DDS::DataReader_ptr get_dds_datareader() const = 0;

    virtual void close() = 0;
};

template <typename T>
class DRHolder : public DRHolderBase
{
public:
    DRHolder(const dds::sub::DataReader<T>& dr) : dr_(dr)
    {
        dr_var_ = DDS::DataReader::_narrow(((dds::sub::DataReader<T>)dr)->get_raw_reader());
    }
    virtual ~DRHolder() { }
public:
    virtual const dds::sub::qos::DataReaderQos qos() const
    {
        return dr_.qos();
    }

    virtual void qos(const dds::sub::qos::DataReaderQos& qos)
    {
        dr_.qos(qos);
    }

    virtual const std::string topic_name() const
    {
        return dr_.topic_description().name();
    }

    virtual const std::string type_name() const
    {
        return dr_.topic_description().type_name();
    }

    virtual ::dds::sub::Subscriber parent() const
    {
        /** @internal
        @bug OSPL-2296 Previous code (below) recursed infinitely.
        @code
        return parent();
        @endcode
        @see http://jira.prismtech.com:8080/browse/OSPL-2296 */
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
#ifdef __SUNPRO_CC
        /** @internal
         * @todo Remove this (obviously) once implemented. SunCC is
         * mad as a box of frogs - it warns without the below line because
         * the function doesn't return anything, despite the unconditional
         * throw above */
        return ::dds::sub::Subscriber(dds::core::null);
#endif
    }

    virtual DDS::DataReader_ptr get_dds_datareader() const
    {
        return dr_var_.in();
    }

    virtual void close()
    {
        dr_.close();
    }

    dds::sub::DataReader<T> get() const
    {
        return dr_;
    }

private:
    dds::sub::DataReader<T> dr_;
    DDS::DataReader_var dr_var_;
};
}
}
}

// End of implementation

#endif /* OSPL_DDS_SUB_DETAIL_ANYDATAREADER_HPP_ */
