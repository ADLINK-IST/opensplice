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


/**
 * @file
 */

#include <dds/core/Time.hpp>
#include <dds/core/Duration.hpp>
#include <dds/core/Exception.hpp>
#include <org/opensplice/core/exception_helper.hpp>

#define MS 1000
#define MiS 1000000
#define NS 1000000000

dds::core::Time::Time()
    :  sec_(0L),
       nsec_(0u)
{ }

dds::core::Time::Time(int64_t s, uint32_t ns)
{
    sec(s);
    nanosec(ns);
}

int64_t dds::core::Time::sec() const
{
    return sec_;
}

void dds::core::Time::sec(int64_t s)
{
    if(s < 0 && s != -1)
    {
        std::string message(org::opensplice::core::context_to_string(OSPL_CONTEXT_LITERAL("dds::core::InvalidDataError")));
        message += "dds::core::Time::sec out of bounds";
        throw dds::core::InvalidDataError(org::opensplice::core::exception_helper(message, false));
    }
    else
    {
        /** @internal @bug OSPL-2308 RTF Time-ish coercion issue
        @see http://jira.prismtech.com:8080/browse/OSPL-2308 */
        sec_ =  static_cast<int32_t>(s);
    }
}

uint32_t dds::core::Time::nanosec() const
{
    return nsec_;
}

void dds::core::Time::nanosec(uint32_t ns)
{
    if((ns > NS && ns != 0xffffffff) || (sec() == -1 && ns != 0xffffffff))
    {
        std::string message(org::opensplice::core::context_to_string(OSPL_CONTEXT_LITERAL("dds::core::InvalidDataError")));
        message += "dds::core::Time::nanosec out of bounds";
        throw dds::core::InvalidDataError(org::opensplice::core::exception_helper(message, false));
    }
    else
    {
        nsec_ = ns;
    }
}

const dds::core::Time dds::core::Time::from_microsecs(int64_t microseconds)
{
    return Time(microseconds / MiS, static_cast<uint32_t>((microseconds % MiS) * MS));
}

const dds::core::Time dds::core::Time::from_millisecs(int64_t milliseconds)
{
    return Time(milliseconds / MS, static_cast<uint32_t>((milliseconds % MS) * MiS));
}

const dds::core::Time dds::core::Time::from_secs(double seconds)
{
    int64_t int_secs =  static_cast<int64_t>(seconds);
    uint32_t nanos = static_cast<uint32_t>((seconds - int_secs) * NS);
    return Time(int_secs, nanos);
}

int dds::core::Time::compare(const Time& that) const
{
    int ret;

    if(sec_ >= that.sec_ && (sec_ > that.sec_ || nsec_ > that.nsec_))
    {
        ret = 1;
    }
    else if(sec_ <= that.sec_ && (sec_ < that.sec_ || nsec_ < that.nsec_))
    {
        ret = -1;
    }
    else
    {
        ret = 0;
    }

    return ret;
}

bool
dds::core::Time::operator >(const Time& that) const
{
    return sec_ >= that.sec_ && (sec_ > that.sec_ || nsec_ > that.nsec_);
}

bool
dds::core::Time::operator >=(const Time& that) const
{
    return sec_ >= that.sec_ && nsec_ >= that.nsec_;
}

bool
dds::core::Time::operator ==(const Time& that) const
{
    return sec_ == that.sec_ && nsec_ == that.nsec_;
}
bool
dds::core::Time::operator <=(const Time& that) const
{
    return sec_ <= that.sec_ && nsec_ <= that.nsec_;
}
bool
dds::core::Time::operator <(const Time& that) const
{
    return sec_ <= that.sec_ && (sec_ < that.sec_ || nsec_ < that.nsec_);
}

dds::core::Time& dds::core::Time::operator+=(const Duration& a_ti)
{
    org::opensplice::core::validate<dds::core::Time>(*this, OSPL_CONTEXT_LITERAL(""));
    org::opensplice::core::validate<dds::core::Duration>(a_ti, OSPL_CONTEXT_LITERAL(""));
    /** @internal @bug OSPL-2308 RTF Time-ish coercion issue
        @see http://jira.prismtech.com:8080/browse/OSPL-2308 */
    this->sec_ += static_cast<int32_t>(a_ti.sec());
    uint32_t dns = this->nsec_ + a_ti.nanosec();
    if(dns > NS)
    {
        this->sec_++;
        this->nsec_ = dns % NS;
    }
    else
    {
        nsec_ = dns;
    }
    return *this;
}

dds::core::Time& dds::core::Time::operator-=(const Duration& a_ti)
{
    org::opensplice::core::validate<dds::core::Time>(*this, OSPL_CONTEXT_LITERAL(""));
    org::opensplice::core::validate<dds::core::Duration>(a_ti, OSPL_CONTEXT_LITERAL(""));
    try
    {
        dds::core::Time tmp(sec_ - a_ti.sec());
        uint32_t dns = a_ti.nanosec();
        uint32_t tmpNS;
        if(nsec_ < dns)
        {
            tmp.sec(tmp.sec() - 1);
            tmpNS = nsec_ + NS - dns;
        }
        else
        {
            tmpNS = nsec_ - dns;
        }
        tmp.nanosec(tmpNS);
        org::opensplice::core::validate<dds::core::Time>(tmp, OSPL_CONTEXT_LITERAL(""));
        this->nanosec(tmp.nanosec());
        this->sec(tmp.sec());
    }
    catch(std::exception& e)
    {
        std::string message(org::opensplice::core::context_to_string(OSPL_CONTEXT_LITERAL("dds::core::IllegalOperationError")));
        message += " Arithmetic operation resulted in a out of bounds";
        message += "\n";
        message += e.what();
        throw dds::core::InvalidDataError(org::opensplice::core::exception_helper(message, false));
    }
    return *this;
}

int64_t dds::core::Time::to_millisecs() const
{
    org::opensplice::core::validate<dds::core::Time>(*this, OSPL_CONTEXT_LITERAL(""));
    return (static_cast<int64_t>(sec_) * MS) + (nsec_ / MiS);
}

int64_t dds::core::Time::to_microsecs() const
{
    org::opensplice::core::validate<dds::core::Time>(*this, OSPL_CONTEXT_LITERAL(""));
    return (static_cast<int64_t>(sec_) * MiS) + (nsec_ / MS);
}

double dds::core::Time::to_secs() const
{
    org::opensplice::core::validate<dds::core::Time>(*this, OSPL_CONTEXT_LITERAL(""));
    return static_cast<double>(sec_) + (static_cast<double>(nsec_) / NS);
}

const dds::core::Time operator+(const dds::core::Time& lhs, const dds::core::Duration& rhs)
{
    return dds::core::Time(lhs.sec(), lhs.nanosec()) += rhs;
}

const dds::core::Time operator +(const dds::core::Duration& lhs, const dds::core::Time& rhs)
{
    return dds::core::Time(rhs.sec(), rhs.nanosec()) += lhs;
}

const dds::core::Time operator -(const dds::core::Time& lhs, const dds::core::Duration& rhs)
{
    return dds::core::Time(lhs.sec(), lhs.nanosec()) -= rhs;
}
