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
*
*/


/**
 * @file
 */

#include <org/opensplice/core/TimeHelper.hpp>
#include <org/opensplice/core/ReportUtils.hpp>
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

dds::core::Duration::Duration(int32_t s, uint32_t ns)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    /* Use setter functions to validate values. */
    this->sec(s);
    this->nanosec(ns);
}

#if __cplusplus >= 199711L
dds::core::Duration::Duration(int64_t s, uint32_t ns)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    /* Use setter functions to validate values. */
    this->sec(s);
    this->nanosec(ns);
}
#endif

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
    ISOCPP_REPORT_STACK_NC_BEGIN();

    if(s < 0) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "dds::core::Duration::sec out of bounds");
    } else {
        /** @internal @bug OSPL-2308 RTF Time-ish coercion issue */
        sec_ = static_cast<int32_t>(s);
    }
}

uint32_t dds::core::Duration::nanosec() const
{
    return nsec_;
}

void dds::core::Duration::nanosec(uint32_t ns)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    if((ns > NS) && (ns != 0x7fffffff)) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "dds::core::Duration::nanosec out of bounds");
    } else {
        nsec_ = ns;
    }
}

const dds::core::Duration dds::core::Duration::from_microsecs(int64_t microseconds)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    return Duration(microseconds / MiS, (microseconds % MiS) * MS);
}

const dds::core::Duration dds::core::Duration::from_millisecs(int64_t milliseconds)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    return Duration(milliseconds / MS, (milliseconds % MS) * MiS);
}

const dds::core::Duration dds::core::Duration::from_secs(double seconds)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    int64_t int_secs =  static_cast<int64_t>(seconds);
    uint32_t nanos = static_cast<uint32_t>((seconds - int_secs) * NS);
    return Duration(int_secs, nanos);
}

int dds::core::Duration::compare(const Duration& that) const
{
    int ret;
    if(*this > that) {
        ret = 1;
    } else if(*this < that) {
        ret = -1;
    } else {
        ret = 0;
    }
    return ret;
}

int64_t dds::core::Duration::to_millisecs() const
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    org::opensplice::core::timehelper::validate<dds::core::Duration>(*this, "dds::core::Duration", "to_millisecs");
    return (static_cast<int64_t>(sec_) * MS) + (nsec_ / MiS);
}

int64_t dds::core::Duration::to_microsecs() const
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    org::opensplice::core::timehelper::validate<dds::core::Duration>(*this, "dds::core::Duration", "to_microsecs");
    return (static_cast<int64_t>(sec_) * MiS) + (nsec_ / MS);
}


double dds::core::Duration::to_secs() const
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    org::opensplice::core::timehelper::validate<dds::core::Duration>(*this, "dds::core::Duration", "to_secs");
    return static_cast<double>(sec_) + (static_cast<double>(nsec_) / NS);
}

bool
dds::core::Duration::operator >(const Duration& that) const
{
    return (sec_ > that.sec_) || ((sec_ == that.sec_) && (nsec_ > that.nsec_));
}

bool
dds::core::Duration::operator >=(const Duration& that) const
{
    return !(*this < that);
}

bool
dds::core::Duration::operator !=(const Duration& that) const
{
    return !(*this == that);
}

bool
dds::core::Duration::operator ==(const Duration& that) const
{
    return (sec_ == that.sec_) && (nsec_ == that.nsec_);
}

bool
dds::core::Duration::operator <=(const Duration& that) const
{
    return !(*this > that);
}

bool
dds::core::Duration::operator <(const Duration& that) const
{
    return (sec_ < that.sec_) || ((sec_ == that.sec_) && (nsec_ < that.nsec_));
}

dds::core::Duration&
dds::core::Duration::operator +=(const Duration& a_ti)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    org::opensplice::core::timehelper::validate<dds::core::Duration>(*this, "dds::core::Duration", "operator += this");
    org::opensplice::core::timehelper::validate<dds::core::Duration>(a_ti, "dds::core::Duration", "operator += a_ti");
    sec_ += a_ti.sec();
    uint32_t dns = nsec_ + a_ti.nanosec();
    if(dns > NS) {
        sec_++;
        nsec_ = dns % NS;
    } else {
        nsec_ = dns;
    }
    return *this;
}

dds::core::Duration&
dds::core::Duration::operator -=(const Duration& a_ti)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    org::opensplice::core::timehelper::validate<dds::core::Duration>(*this, "dds::core::Duration", "operator -= this");
    org::opensplice::core::timehelper::validate<dds::core::Duration>(a_ti, "dds::core::Duration", "operator -= a_ti");
    try {
        dds::core::Duration tmp(sec_ - a_ti.sec());
        uint32_t dns = a_ti.nanosec();
        uint32_t tmpNS;
        if(nsec_ < dns) {
            tmp.sec(tmp.sec() - 1);
            tmpNS = nsec_ + NS - dns;
        } else {
            tmpNS = nsec_ - dns;
        }
        tmp.nanosec(tmpNS);
        org::opensplice::core::timehelper::validate<dds::core::Duration>(tmp, "dds::core::Duration", "operator -= tmp");
        this->nanosec(tmp.nanosec());
        this->sec(tmp.sec());
    } catch (dds::core::Exception& e) {
            throw;
    } catch(std::exception& e) {
        std::string message("dds::core::Duration::operator -= IllegalOperationError");
        message += " Arithmetic operation resulted in a out of bounds";
        message += "\n";
        message += e.what();
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, message.c_str());
    }

    return *this;
}

const dds::core::Duration dds::core::Duration::operator +(const Duration& other) const
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    return(Duration(sec_, nsec_) += other);
}

const dds::core::Duration dds::core::Duration::operator -(const Duration& other) const
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    return (Duration(sec_, nsec_) -= other);
}

dds::core::Duration&
dds::core::Duration::operator *=(uint64_t factor)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    org::opensplice::core::timehelper::validate<dds::core::Duration>(*this, "dds::core::Duration", " operator *=");
    this->sec(this->sec_ * factor);
    uint64_t ns = this->nanosec() * factor;
    if(ns  > NS) {
        this->sec(this->sec_ + ns / NS);
        /* cast below is safe because ns % NS is always < NS which is 32 bit */
        this->nanosec(static_cast<uint32_t>(ns % NS));
    } else {
        /* cast below is safe necause ns < NS in this clause */
        this->nanosec(static_cast<uint32_t>(ns));
    }
    return *this;
}

const dds::core::Duration operator *(uint64_t lhs, const dds::core::Duration& rhs)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    return dds::core::Duration(rhs.sec(), rhs.nanosec()) *= lhs;
}

const dds::core::Duration operator *(const dds::core::Duration& lhs, uint64_t rhs)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    return dds::core::Duration(lhs.sec(), lhs.nanosec()) *= rhs;
}

const dds::core::Duration operator /(const dds::core::Duration& rhs, uint64_t lhs)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    org::opensplice::core::timehelper::validate<dds::core::Duration>(rhs, "dds::core::Duration", " operator /");
    /* cast below is safe because rhs.nanosec() < NS which is 32 bit.
     * Any truncation of an lhs > 32 bit will still > NS and result correctly in 0
     */
    return dds::core::Duration(static_cast<int64_t>(rhs.sec() / lhs), static_cast<uint32_t>(rhs.nanosec() / lhs));
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
