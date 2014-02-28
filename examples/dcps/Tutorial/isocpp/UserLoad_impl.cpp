
/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#include "UserLoad_impl.hpp"
#include "common/example_utilities.h"

#include "Entities.cpp"

namespace examples {
#ifdef GENERATING_EXAMPLE_DOXYGEN
GENERATING_EXAMPLE_DOXYGEN /* workaround doxygen bug */
#endif
namespace dcps { namespace Tutorial {

/**
 * @addtogroup examplesdcpsTutorialisocpp The ISO C++ DCPS API Tutorial example
 * The UserLoad program utilises WaitSets and two DataReaders in order to detect
 * when a user has joined or left the message board. It will display the username
 * of a user who joins and will use a Query to display the number of messages sent
 * by a user who leaves.
 *
 * @ingroup examplesdcpsisocpp
 */
/** @{*/
/** @dir */
/** @file */

namespace isocpp  {

/**
 * The NewUserHandler will be called when the ReadCondition triggers.
 * It will output a message showing the user name of the user that has joined.
 */
class NewUserHandler
{
public:
    /**
     * @param dataState The dataState on which to filter the messages
     */
    NewUserHandler(dds::sub::status::DataState& dataState)
    : dataState(dataState) {}

    void operator() (dds::sub::DataReader<Chat::NameService>& dr)
    {
        /** Read the new users */
        dds::sub::LoanedSamples<Chat::NameService> messages = dr.select().state(dataState).read();

        /** Output each new user's user name */
        for (dds::sub::LoanedSamples<Chat::NameService>::const_iterator message = messages.begin();
            message < messages.end(); ++message)
        {
            if(message->info().valid())
            {
                std::cout << "New user: " << message->data().name() << std::endl;
            }
        }
    }

private:
    dds::sub::status::DataState& dataState;
};

/**
 * The UserLeftHandler will be called when the StatusCondition triggers.
 * It will output a message showing users that have left and the number of messages
 * they have sent.
 */
class UserLeftHandler
{
public:
    /**
     * @param nameServiceReader the dds::sub::DataReader<Chat::NameService>
     */
    UserLeftHandler(bool& terminated, dds::sub::DataReader<Chat::NameService>& nameServiceReader)
        : terminated(terminated), nameServiceReader(nameServiceReader), prevAliveCount(0) {}

    void operator() (dds::core::Entity& e)
    {
        dds::sub::DataReader<Chat::ChatMessage>& chatMessageReader
            = (dds::sub::DataReader<Chat::ChatMessage>&)e;

        /** Check that the liveliness has changed */
        const dds::core::status::LivelinessChangedStatus livChangedStatus
            = chatMessageReader.liveliness_changed_status();

        if(livChangedStatus.alive_count() < prevAliveCount)
        {
            /** Take inactive users so they will not appear in the user list */
            dds::sub::status::DataState notAliveDataState;
            notAliveDataState << dds::sub::status::SampleState::any()
                    << dds::sub::status::ViewState::any()
                    << dds::sub::status::InstanceState::not_alive_mask();
            dds::sub::LoanedSamples<Chat::NameService> users
                = nameServiceReader.select().state(notAliveDataState).take();

             /** Output the number of messages sent by each user that has left */
            for (dds::sub::LoanedSamples<Chat::NameService>::const_iterator user = users.begin();
                user < users.end(); ++user)
            {
                if(user->info().valid())
                {
                    /** Get messages sent by the user that has left by userID */
                    std::stringstream queryString;
                    queryString << "userID=" << user->data().userID();
                    dds::sub::Query query(chatMessageReader, queryString.str());
                    dds::sub::LoanedSamples<Chat::ChatMessage> messages
                        = chatMessageReader.select().content(query).take();

                    std::cout << "Departed user " << user->data().name()
                        << " has sent " << messages.length() << " messages." << std::endl;

                    if(user->data().userID() == TERMINATION_MESSAGE)
                    {
                        std::cout << "Termination message received: exiting..." << std::endl;
                        terminated = true;
                    }
                }
            }
        }
        prevAliveCount = livChangedStatus.alive_count();
    }

private:
    bool& terminated;
    dds::sub::DataReader<Chat::NameService>& nameServiceReader;
    int prevAliveCount;
};

/**
 * The EscapeHandler will be called when the GuardCondition triggers.
 * It will cause the program to terminate.
 */
class EscapeHandler
{
public:
    EscapeHandler(bool& terminated) : terminated(terminated) {}

    void operator() (void)
    {
        std::cout << "UserLoad has terminated." << std::endl;
        terminated = true;
    }

private:
    bool& terminated;
};

/** A GuardCondition is created */
dds::core::cond::GuardCondition escape;

/**
 * This function will be executed in a thread and will trigger the GuardCondition
 * after 60 seconds.
 */
extern "C" void* delayedEscape(void *arg)
{
    exampleSleepMilliseconds(60000);
    escape.trigger_value(true);

    return NULL;
}

/**
 * This function performs the UserLoad role in this example.
 * @return 0 if successful, 1 otherwise.
 */
int UserLoad(int argc, char *argv[])
{
    int result = 0;
    bool terminated = false;
    try
    {
        /** Initialise entities */
        SubEntities e;

        /**
         * A ReadCondition is created and assigned a handler which is triggered
         * when a new user joins
         */
        dds::sub::status::DataState newDataState;
        newDataState << dds::sub::status::SampleState::not_read()
                << dds::sub::status::ViewState::new_view()
                << dds::sub::status::InstanceState::alive();
        NewUserHandler newUserHandler(newDataState);
        dds::sub::cond::ReadCondition newUser(e.nameServiceReader, newDataState, newUserHandler);

        /**
         * A StatusCondition is created and assigned a handler which is triggered
         * when a DataWriter changes it's liveliness
         */
        UserLeftHandler userLeftHandler(terminated, e.nameServiceReader);
        dds::core::cond::StatusCondition userLeft(e.chatMessageReader, userLeftHandler);
        dds::core::status::StatusMask statusMask;
        statusMask << dds::core::status::StatusMask::liveliness_changed();
        userLeft.enabled_statuses(statusMask);

        /**
         * A GuardCondition is assigned a handler which will be used to close
         * the message board
         */
        EscapeHandler escapeHandler(terminated);
        escape.handler(escapeHandler);

        /** A WaitSet is created and the four conditions created above are attached to it */
        dds::core::cond::WaitSet waitSet;
        waitSet += newUser;
        waitSet += userLeft;
        waitSet += escape;

        /** Take inactive users so they will not appear in the user list */
        dds::sub::status::DataState notAliveDataState;
        notAliveDataState << dds::sub::status::SampleState::any()
                << dds::sub::status::ViewState::any()
                << dds::sub::status::InstanceState::not_alive_mask();
        e.nameServiceReader.select().state(notAliveDataState).take();

        /** Start a thread which will trigger the GuardCondition after 60 seconds */
        os_threadId threadId;
        os_threadAttr threadAttr;
        os_threadAttrInit(&threadAttr);
        os_threadCreate(&threadId, "delayedEscape", &threadAttr, delayedEscape, 0);

        std::cout << "UserLoad has opened: disconnect a Chatter with userID = "
            << TERMINATION_MESSAGE << " to close it....\n" << std::endl;

        /**
         * Repeatedly call dispatch on the WaitSet which will wait for a Condition
         * to trigger and then call it's handler
         */
        while(!terminated)
        {
            waitSet.dispatch();
        }
    }
    catch (const dds::core::Exception& e)
    {
        std::cerr << "ERROR: Exception: " << e.what() << std::endl;
        result = 1;
    }
    return result;
}

}
}
}
}

EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_Tutorial_user_load, examples::dcps::Tutorial::isocpp::UserLoad)
