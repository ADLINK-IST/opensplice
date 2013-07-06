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

#ifndef ORG_OPENSPLICE_CORE_WAITSETIMPL_HPP_
#define ORG_OPENSPLICE_CORE_WAITSETIMPL_HPP_

#include <vector>
#include <iostream>
#include <algorithm>

#include <org/opensplice/core/exception_helper.hpp>
#include <org/opensplice/core/config.hpp>
#include <dds/core/Duration.hpp>
#include <dds/core/cond/Condition.hpp>

namespace org
{
namespace opensplice
{
namespace core
{

/**
 * The WaitSetImpl class provides the implementation of the functionality of WaitSets.
 * The class largely acts as a wrapper around the DDS WaitSet class (Formal/07-01-01 7.1.2.1.6).
 * It does however add extra functionality via the dispatch methods in the form of functors
 * which allow a handler to be attached to a Condition which is then called when the
 * correspinding condition triggers.
 */
class WaitSetImpl
{
public:
    typedef std::vector<dds::core::cond::Condition> ConditionSeqType;

public:
    WaitSetImpl() { }

    /**
     * The destructor of WaitSetImpl will automatically detach any conditions attached to the
     * WaitSet, removing the requirement for the user to do this manually.
     */
    ~WaitSetImpl()
    {
        for(ConditionSeqType::iterator i = cond_vec_.begin(); i < cond_vec_.end(); i++)
        {
            DDS::ReturnCode_t result = waitset_.detach_condition((*i)->get_dds_condition());
            org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::detach_condition"));
        }
    }
    void close();

public:
    const ConditionSeqType wait(const dds::core::Duration& timeout)
    {
        ConditionSeqType triggered;
        return wait(triggered, timeout);
    }

    const ConditionSeqType wait()
    {
        return wait(dds::core::Duration::infinite());
    }

    /**
     * The wait member function acts as a wrapper around the equivalent DDS member function
     * (Formal/07-01-01 7.1.2.1.6.3). It performs neccesary conversion between DDS and ISOCPP
     * api's for both the timeout value and the returned vector of triggered Conditions.
     *
     * @param triggered A ConditionSeqType vector in which to put the triggered conditions
     * @param timeout How long the waitset should wait
     */
    ConditionSeqType& wait(ConditionSeqType& triggered,
                           const dds::core::Duration& timeout)
    {
        //Convert timeout to DDS api and then wait
        DDS::ConditionSeq cseq;
        DDS::Duration_t ddstimeout;
        /** @bug OSPL-2308 RTF Time-ish coercion issue
            @see http://jira.prismtech.com:8080/browse/OSPL-2308 */
        ddstimeout.sec = static_cast<DDS::Long> (timeout.sec());
        ddstimeout.nanosec = timeout.nanosec();
        DDS::ReturnCode_t result = waitset_.wait(cseq, ddstimeout);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::wait"));

        //Convert triggered conditions to ISOCPP api and return them
        triggered.clear();
        for(DDS::ULong i = 0; i < cseq.length(); i++)
        {
            for(ConditionSeqType::iterator j = cond_vec_.begin(); j <  cond_vec_.end(); j++)
            {
                if(cseq[i] == (*j)->get_dds_condition())
                {
                    triggered.push_back(*j);
                    break;
                }
            }
        }
        return triggered;
    }

    ConditionSeqType& wait(ConditionSeqType& triggered)
    {
        return wait(triggered, dds::core::Duration::infinite());
    }

public:
    void dispatch()
    {
        dispatch(dds::core::Duration::infinite());
    }

    /**
     * The dispatch member function first calls the wait member function and then for each
     * triggered condition returned it will call their dipatch member function. This will
     * in turn excecute the handler attached to the condition.
     *
     * @param timeout How long the waitset should wait
     */
    void dispatch(const dds::core::Duration& timeout)
    {
        ConditionSeqType triggered = wait(timeout);
        for (ConditionSeqType::iterator i = triggered.begin(); i < triggered.end(); i++)
        {
            (*i)->dispatch();
        }
    }

public:
    /**
     * Gets the associated DDS condition of an ISOCPP condition passed in as a parameter
     * and attaches it to the DDS waitset (Formal/07-01-01 7.1.2.1.6.1), the ISOCPP
     * condition is then stored in a private vector.
     *
     * @param cond The condition to attach
     */
    WaitSetImpl& attach_condition(const dds::core::cond::Condition& cond)
    {
        DDS::ReturnCode_t result = waitset_.attach_condition(cond->get_dds_condition());
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::attach_condition"));
        cond_vec_.push_back(cond);
        return *this;
    }

    /**
     * Compares the triggered passed in condition by DDS reference to those in the stored conditions
     * and detaches (Formal/07-01-01 7.1.2.1.6.2) the one from the DDS waitset that is equal.
     */
    bool detach_condition(const dds::core::cond::Condition& cond)
    {
        DDS::ReturnCode_t result = waitset_.detach_condition(cond->get_dds_condition());
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::detach_condition"));
        for(ConditionSeqType::iterator i = cond_vec_.begin(); i < cond_vec_.end(); i++)
        {
            if((*i)->get_dds_condition() == cond->get_dds_condition())
            {
                cond_vec_.erase(i);
                return true;
            }
        }
        return false;
    }

    const ConditionSeqType& conditions()
    {
        return cond_vec_;
    }

    ConditionSeqType& conditions(ConditionSeqType& conds) const
    {
        conds.assign(cond_vec_.begin(), cond_vec_.end());
        return conds;
    }

private:
    DDS::WaitSet waitset_;
    ConditionSeqType cond_vec_;
};

}
}
}

#endif /* ORG_OPENSPLICE_CORE_WAITSETIMPL_HPP_ */
