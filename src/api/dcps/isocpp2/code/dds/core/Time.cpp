/*
*                         OpenSplice DDS
*
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#include <dds/core/Time.hpp>
#include <dds/core/Duration.hpp>
#include <dds/core/Exception.hpp>

#define MS 1000
#define MiS 1000000
#define NS 1000000000


const dds::core::Time
dds::core::Time::invalid()
{
    static const Time inv(-1, 0x7fffffff);

    return inv;
}

dds::core::Time::Time()
    :  sec_(0L),
       nsec_(0u)
{ }

dds::core::Time::Time(int64_t s, uint32_t ns)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    /* Use setter functions to validate values. */
    this->sec(s);
    this->nanosec(ns);
}

int64_t dds::core::Time::sec() const
{
    return sec_;
}

void dds::core::Time::sec(int64_t s)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    if(s < 0 && s != -1) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "dds::core::Time::sec out of bounds");
    } else {
        /** @internal @bug OSPL-2308 RTF Time-ish coercion issue
        @see http://jira.prismtech.com:8080/browse/OSPL-2308 */
        sec_ =  s;
    }
}

uint32_t dds::core::Time::nanosec() const
{
    return nsec_;
}

void dds::core::Time::nanosec(uint32_t ns)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    if((ns > NS && ns != 0x7fffffff) || (sec() == -1 && ns != 0x7fffffff)) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "dds::core::Time::nanosec out of bounds");
    } else {
        nsec_ = ns;
    }
}

const dds::core::Time dds::core::Time::from_microsecs(int64_t microseconds)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    return Time(microseconds / MiS, static_cast<uint32_t>((microseconds % MiS) * MS));
}

const dds::core::Time dds::core::Time::from_millisecs(int64_t milliseconds)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    return Time(milliseconds / MS, static_cast<uint32_t>((milliseconds % MS) * MiS));
}

const dds::core::Time dds::core::Time::from_secs(double seconds)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    int64_t int_secs =  static_cast<int64_t>(seconds);
    uint32_t nanos = static_cast<uint32_t>((seconds - int_secs) * NS);
    return Time(int_secs, nanos);
}

int dds::core::Time::compare(const Time& that) const
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

bool
dds::core::Time::operator >(const Time& that) const
{
    return (sec_ > that.sec_) || ((sec_ == that.sec_) && (nsec_ > that.nsec_));
}

bool
dds::core::Time::operator >=(const Time& that) const
{
    return !(*this < that);
}

bool
dds::core::Time::operator !=(const Time& that) const
{
    return !(*this == that);
}

bool
dds::core::Time::operator ==(const Time& that) const
{
    return (sec_ == that.sec_) && (nsec_ == that.nsec_);
}

bool
dds::core::Time::operator <=(const Time& that) const
{
    return !(*this > that);
}

bool
dds::core::Time::operator <(const Time& that) const
{
    return (sec_ < that.sec_) || ((sec_ == that.sec_) && (nsec_ < that.nsec_));
}

dds::core::Time& dds::core::Time::operator +=(const Duration& a_ti)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    org::opensplice::core::timehelper::validate<dds::core::Time>(*this, "dds::core::Time", " operator += time");
    org::opensplice::core::timehelper::validate<dds::core::Duration>(a_ti, "dds::core::Time", " operator += duration");
    this->sec_ += a_ti.sec();
    uint32_t dns = this->nsec_ + a_ti.nanosec();
    if(dns > NS) {
        this->sec_++;
        this->nsec_ = dns % NS;
    } else {
        nsec_ = dns;
    }
    return *this;
}

dds::core::Time& dds::core::Time::operator -=(const Duration& a_ti)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    org::opensplice::core::timehelper::validate<dds::core::Time>(*this, "dds::core::Time", " operator += time");
    org::opensplice::core::timehelper::validate<dds::core::Duration>(a_ti, "dds::core::Time", " operator += duration");
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
        org::opensplice::core::timehelper::validate<dds::core::Time>(*this, "dds::core::Time", " operator += tmp");
        this->nanosec(tmp.nanosec());
        this->sec(tmp.sec());
    } catch (dds::core::Exception& e) {
        throw;
    } catch(std::exception& e) {
        std::string message("dds::core::Time::operator -= IllegalOperationError");
        message += " Arithmetic operation resulted in a out of bounds";
        message += "\n";
        message += e.what();
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, message.c_str());
    }
    return *this;
}

int64_t dds::core::Time::to_millisecs() const
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    org::opensplice::core::timehelper::validate<dds::core::Time>(*this, "dds::core::Time", "to_millisecs");
    return ((sec_ * MS) + (nsec_ / MiS));
}

int64_t dds::core::Time::to_microsecs() const
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    org::opensplice::core::timehelper::validate<dds::core::Time>(*this, "dds::core::Time", "to_microsecs");
    return ((sec_ * MiS) + (nsec_ / MS));
}

double dds::core::Time::to_secs() const
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    org::opensplice::core::timehelper::validate<dds::core::Time>(*this, "dds::core::Time", "to_secs");
    return static_cast<double>(sec_) + (static_cast<double>(nsec_) / NS);
}

const dds::core::Time operator +(const dds::core::Time& lhs, const dds::core::Duration& rhs)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    return dds::core::Time(lhs.sec(), lhs.nanosec()) += rhs;
}

const dds::core::Time operator +(const dds::core::Duration& lhs, const dds::core::Time& rhs)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    return dds::core::Time(rhs.sec(), rhs.nanosec()) += lhs;
}

const dds::core::Time operator -(const dds::core::Time& lhs, const dds::core::Duration& rhs)
{
    ISOCPP_REPORT_STACK_NC_BEGIN();

    return dds::core::Time(lhs.sec(), lhs.nanosec()) -= rhs;
}
