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
#include <map>

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
 *  @internal The WaitSetImpl class provides the implementation of the functionality of WaitSets.
 * The class largely acts as a wrapper around the DDS WaitSet class (Formal/07-01-01 7.1.2.1.6).
 * It does however add extra functionality via the dispatch functions in the form of functors
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
     *  @internal The destructor of WaitSetImpl will automatically detach any conditions attached to the
     * WaitSet, removing the requirement for the user to do this manually.
     */
    ~WaitSetImpl()
    {
        std::map<DDS::Condition_ptr, dds::core::cond::Condition>::iterator i = cond_map_.begin();
        while(i != cond_map_.end())
        {
            detach_condition((i++)->second);
        }
    }
    void close();

public:
    /**
     *  @internal The wait member function acts as a wrapper around the equivalent DDS member function
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
        /** @internal @bug OSPL-2308 RTF Time-ish coercion issue
            @see http://jira.prismtech.com:8080/browse/OSPL-2308 */
        ddstimeout.sec = static_cast<DDS::Long>(timeout.sec());
        ddstimeout.nanosec = timeout.nanosec();
        DDS::ReturnCode_t result = waitset_.wait(cseq, ddstimeout);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::wait"));

        //Convert triggered conditions to ISOCPP api and return them
        triggered.clear();
        for(DDS::ULong i = 0; i < cseq.length(); i++)
        {
            triggered.push_back(cond_map_.find(cseq[i])->second);
        }

        return triggered;
    }
public:
    /**
     *  @internal The dispatch member function first calls the wait member function and then for each
     * triggered condition returned it will call their dipatch member function. This will
     * in turn excecute the handler attached to the condition.
     *
     * @param timeout How long the waitset should wait
     */
    void dispatch(const dds::core::Duration& timeout)
    {
        ConditionSeqType triggered;
        wait(triggered, timeout);
        for(ConditionSeqType::iterator i = triggered.begin(); i < triggered.end(); i++)
        {
            (*i)->dispatch();
        }
    }

public:
    /**
     *  @internal Gets the associated DDS condition of an ISOCPP condition passed in as a parameter
     * and attaches it to the DDS waitset (Formal/07-01-01 7.1.2.1.6.1), the ISOCPP
     * condition is then stored in a private vector.
     *
     * @param cond The condition to attach
     */
    WaitSetImpl& attach_condition(const dds::core::cond::Condition& cond)
    {
        DDS::ReturnCode_t result = waitset_.attach_condition(cond->get_dds_condition());
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::attach_condition"));
        cond_map_.insert(
            std::map<DDS::Condition_ptr, dds::core::cond::Condition>::value_type(cond->get_dds_condition(), cond));
        return *this;
    }

    /**
     *  @internal Compares the triggered passed in condition by DDS reference to those in the stored conditions
     * and detaches (Formal/07-01-01 7.1.2.1.6.2) the one from the DDS waitset that is equal.
     */
    bool detach_condition(const dds::core::cond::Condition& cond)
    {
        DDS::ReturnCode_t result = waitset_.detach_condition(cond->get_dds_condition());
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::detach_condition"));
        cond_map_.erase(cond->get_dds_condition());
        return true;
    }

    ConditionSeqType& conditions(ConditionSeqType& conds) const
    {
        for(std::map<DDS::Condition_ptr, dds::core::cond::Condition>::const_iterator i = cond_map_.begin(); i != cond_map_.end(); i++)
        {
            conds.push_back(i->second);
        }
        return conds;
    }

private:
    DDS::WaitSet waitset_;
    std::map<DDS::Condition_ptr, dds::core::cond::Condition> cond_map_;
};

}
}
}

#endif /* ORG_OPENSPLICE_CORE_WAITSETIMPL_HPP_ */
