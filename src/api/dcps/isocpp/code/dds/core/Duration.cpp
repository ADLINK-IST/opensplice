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

#include <org/opensplice/core/exception_helper.hpp>
#include <dds/core/Exception.hpp>
#include <dds/core/Time.hpp>
#include <dds/core/Duration.hpp>
#define MS 1000
#define MiS 1000000
#define NS 1000000000


dds::core::Duration::Duration()
    :  sec_(0),
       nsec_(0)
{ }

dds::core::Duration::Duration(int64_t s, uint32_t ns)
{
    sec(s);
    nanosec(ns);
}

dds::core::Duration::~Duration()
{
    // implementation-defined
}

int64_t dds::core::Duration::sec() const
{
    return sec_;
}

void dds::core::Duration::sec(int64_t s)
{
    if(s < 0)
    {
        std::string message(org::opensplice::core::context_to_string(OSPL_CONTEXT_LITERAL("dds::core::InvalidDataError")));
        message += " dds::core::Duration::sec out of bounds";
        throw dds::core::InvalidDataError(org::opensplice::core::exception_helper(message, false));
    }
    else
    {
        /** @internal @bug OSPL-2308 RTF Time-ish coercion issue
        @see http://jira.prismtech.com:8080/browse/OSPL-2308 */
        sec_ = static_cast<int32_t>(s);
    }
}

void dds::core::Duration::nanosec(uint32_t ns)
{
    if(ns > NS && ns != 0x7fffffff)
    {
        std::string message(org::opensplice::core::context_to_string(OSPL_CONTEXT_LITERAL("dds::core::InvalidDataError")));
        message += " dds::core::Duration::nanosec out of bounds";
        throw dds::core::InvalidDataError(org::opensplice::core::exception_helper(message, false));
    }
    else
    {
        nsec_ = ns;
    }
}

const dds::core::Duration dds::core::Duration::from_microsecs(int64_t microseconds)
{
    return Duration(microseconds / MiS, static_cast<uint32_t>((microseconds % MiS) * MS));
}

const dds::core::Duration dds::core::Duration::from_millisecs(int64_t milliseconds)
{
    return Duration(milliseconds / MS, static_cast<uint32_t>((milliseconds % MS) * MiS));
}

const dds::core::Duration dds::core::Duration::from_secs(double seconds)
{
    int64_t int_secs =  static_cast<int64_t>(seconds);
    uint32_t nanos = static_cast<uint32_t>((seconds - int_secs) * NS);
    return Duration(int_secs, nanos);
}

uint32_t dds::core::Duration::nanosec() const
{
    return nsec_;
}

int dds::core::Duration::compare(const Duration& that) const
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

int64_t dds::core::Duration::to_millisecs() const
{
    org::opensplice::core::validate<dds::core::Duration>(*this, OSPL_CONTEXT_LITERAL(""));
    return (static_cast<int64_t>(sec_) * MS) + (nsec_ / MiS);
}

int64_t dds::core::Duration::to_microsecs() const
{
    org::opensplice::core::validate<dds::core::Duration>(*this, OSPL_CONTEXT_LITERAL(""));
    return (static_cast<int64_t>(sec_) * MiS) + (nsec_ / MS);
}


double dds::core::Duration::to_secs() const
{
    org::opensplice::core::validate<dds::core::Duration>(*this, OSPL_CONTEXT_LITERAL(""));
    return static_cast<double>(sec_) + (static_cast<double>(nsec_) / NS);
}

bool
dds::core::Duration::operator >(const Duration& that) const
{
    return sec_ >= that.sec_ && (sec_ > that.sec_ || nsec_ > that.nsec_);
}

bool
dds::core::Duration::operator >=(const Duration& that) const
{
    return sec_ >= that.sec_ && nsec_ >= that.nsec_;
}

bool
dds::core::Duration::operator ==(const Duration& that) const
{
    return sec_ == that.sec_ && nsec_ == that.nsec_;
}

bool
dds::core::Duration::operator <=(const Duration& that) const
{
    return sec_ <= that.sec_ && nsec_ <= that.nsec_;
}

bool
dds::core::Duration::operator <(const Duration& that) const
{
    return sec_ <= that.sec_ && (sec_ < that.sec_ || nsec_ < that.nsec_);
}

dds::core::Duration&
dds::core::Duration::operator +=(const Duration& a_ti)
{
    org::opensplice::core::validate<dds::core::Duration>(*this, OSPL_CONTEXT_LITERAL(""));
    org::opensplice::core::validate<dds::core::Duration>(a_ti, OSPL_CONTEXT_LITERAL(""));
    /** @internal @bug OSPL-2308 RTF Time-ish coercion issue
        @see http://jira.prismtech.com:8080/browse/OSPL-2308 */
    sec_ += static_cast<int32_t>(a_ti.sec());
    uint32_t dns = nsec_ + a_ti.nanosec();
    if(dns > NS)
    {
        sec_++;
        nsec_ = dns % NS;
    }
    else
    {
        nsec_ = dns;
    }
    return *this;
}

dds::core::Duration&
dds::core::Duration::operator -=(const Duration& a_ti)
{
    org::opensplice::core::validate<dds::core::Duration>(*this, OSPL_CONTEXT_LITERAL(""));
    org::opensplice::core::validate<dds::core::Duration>(a_ti, OSPL_CONTEXT_LITERAL(""));
    try
    {
        dds::core::Duration tmp(sec_ - a_ti.sec());
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
        org::opensplice::core::validate<dds::core::Duration>(tmp, OSPL_CONTEXT_LITERAL(""));
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

const dds::core::Duration dds::core::Duration::operator +(const Duration& other) const
{
    return(Duration(sec_, nsec_) += other);
}

const dds::core::Duration dds::core::Duration::operator -(const Duration& other) const
{
    return (Duration(sec_, nsec_) -= other);
}

dds::core::Duration&
dds::core::Duration::operator *=(uint64_t factor)
{
    org::opensplice::core::validate<dds::core::Duration>(*this, OSPL_CONTEXT_LITERAL(""));
    this->sec(this->sec_ * factor);
    uint64_t ns = this->nanosec() * factor;
    if(ns  > NS)
    {
        this->sec(this->sec_ + ns / NS);
        /* cast below is safe because ns % NS is always < NS which is 32 bit */
        this->nanosec(static_cast<uint32_t>(ns % NS));
    }
    else
    {
        /* cast below is safe necause ns < NS in this clause */
        this->nanosec(static_cast<uint32_t>(ns));
    }
    return *this;
}

const dds::core::Duration operator *(uint64_t lhs, const dds::core::Duration& rhs)
{
    return dds::core::Duration(rhs.sec(), rhs.nanosec()) *= lhs;
}

const dds::core::Duration operator *(const dds::core::Duration& lhs, uint64_t rhs)
{
    return dds::core::Duration(lhs.sec(), lhs.nanosec()) *= rhs;
}

const dds::core::Duration operator /(const dds::core::Duration& rhs, uint64_t lhs)
{
    org::opensplice::core::validate<dds::core::Duration>(rhs);
    /* cast below is safe because rhs.nanosec() < NS which is 32 bit.
    Any truncation of an lhs > 32 bit will still > NS and result correctly in 0 */
    return dds::core::Duration((rhs.sec() / lhs), (rhs.nanosec() / static_cast<uint32_t>(lhs)));
}

const dds::core::Duration
dds::core::Duration::infinite()
{
    static Duration d(0x7fffffff, 0x7fffffff);
    return d;
}

const dds::core::Duration
dds::core::Duration::zero()
{
    static Duration d(0, 0);
    return d;
}
