
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

#include "MessageBoard_impl.hpp"
#include "common/example_utilities.h"

#include "Entities.cpp"

#define TERMINATION_MESSAGE -1

namespace examples {
#ifdef GENERATING_EXAMPLE_DOXYGEN
GENERATING_EXAMPLE_DOXYGEN /* workaround doxygen bug */
#endif
namespace dcps { namespace Tutorial {

/**
 * @addtogroup examplesdcpsTutorialisocpp The ISO C++ DCPS API Tutorial example
 * The MessageBoard program utilises Queries, DataStates and two DataReaders in
 * order to read messages from the system and display the associated username
 * of the sender as well as filtering out an ignored user.
 *
 * @ingroup examplesdcpsisocpp
 */
/** @{*/
/** @dir */
/** @file */

namespace isocpp  {

/**
 * This function performs the MessageBoard role in this example.
 * @return 0 if successful, 1 otherwise.
 */
int MessageBoard(int argc, char *argv[])
{
    int result = 0;
    try
    {
        /** Initialise entities */
        SubEntities e;

        /** Get the userID to ignore from the program parameters */
        /** Parameters: MessageBoard [ignoreUserID] */
        int ignoreUserID = 0;
        if(argc > 1)
        {
            ignoreUserID = atoi(argv[1]);
        }

        std::cout
            << "MessageBoard has opened: send a ChatMessage with userID = -1 to close it....\n"
            << std::endl;

        bool terminated = false;
        while(!terminated)
        {
            /** Create a DataState which will ensure the take onlys take alive messages */
            dds::sub::status::DataState aliveDataState;
            aliveDataState << dds::sub::status::SampleState::any()
                << dds::sub::status::ViewState::any()
                << dds::sub::status::InstanceState::alive();

            /**
             * Create a Query to ignore messages from the userID taken from the program
             * parameters
             */
            std::stringstream queryString;
            queryString << "userID<>" << ignoreUserID;
            dds::sub::Query ignoreUserIDQuery(e.nameServiceReader, queryString.str());

            /**
             * Take messages. Using take instead of read removes the messages from
             * the system, preventing resources from being saturated due to a build
             * up of messages
             */
            dds::sub::LoanedSamples<Chat::ChatMessage> messages
                = e.chatMessageReader.select().content(ignoreUserIDQuery)
                    .state(aliveDataState).take();

            /** Output the username and content for each message */
            for (dds::sub::LoanedSamples<Chat::ChatMessage>::const_iterator message
                    = messages.begin(); message < messages.end(); ++message)
            {
                if(message->info().valid())
                {
                    /** Terminate if termination message is received */
                    if(message->data().userID() == TERMINATION_MESSAGE)
                    {
                        std::cout << "Termination message received: exiting..." << std::endl;
                        terminated = true;
                    }
                    else
                    {
                        /** Create a Query to get the user with the userID from the message */
                        std::stringstream queryString;
                        queryString << "userID=" << message->data().userID();
                        dds::sub::Query userIDQuery(e.nameServiceReader, queryString.str());

                        /** Get the user */
                        dds::sub::LoanedSamples<Chat::NameService> users
                            = e.nameServiceReader.select().content(userIDQuery)
                                .state(aliveDataState).read();

                        /** Output the username and content */
                        for (dds::sub::LoanedSamples<Chat::NameService>::const_iterator user
                                = users.begin(); user < users.end(); ++user)
                        {
                            if(user->info().valid())
                            {
                                std::cout << user->data().name() << ": "
                                    << message->data().content() << std::endl;
                            }
                        }
                    }
                }
            }
            /** Sleep to avoid utilising too much CPU */
            exampleSleepMilliseconds(100);
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

EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_Tutorial_MessageBoard, examples::dcps::Tutorial::isocpp::MessageBoard)
