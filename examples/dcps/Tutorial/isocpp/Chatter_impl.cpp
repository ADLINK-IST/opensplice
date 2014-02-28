
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

#include "Chatter_impl.hpp"
#include "common/example_utilities.h"

#include "Entities.cpp"

namespace examples {
#ifdef GENERATING_EXAMPLE_DOXYGEN
GENERATING_EXAMPLE_DOXYGEN /* workaround doxygen bug */
#endif
namespace dcps { namespace Tutorial {

/**
 * @addtogroup examplesdcpsTutorialisocpp The ISO C++ DCPS API Tutorial example
 * The Chatter program utilises two DataWriters in order to create a new user
 * within the system and write a number of messages sent by the user.
 *
 * @ingroup examplesdcpsisocpp
 */
/** @{*/
/** @dir */
/** @file */

namespace isocpp  {

/**
 * This function performs the Chatter role in this example
 * @return 0 if successful, 1 otherwise.
 */
int Chatter(int argc, char *argv[])
{
    int result = 0;
    std::stringstream ss;
    try
    {
        /** Initialise entities */
        PubEntities e;

        int numMsg = 10;

        /**
         * Get the user details from the program parameters and write them to the system
         * Parameters: Chatter [userID] [userName] [numMsg]
         */
        Chat::NameService user;
        user.userID() = argc < 2 ? 1 : atoi(argv[1]);
        if(argc < 3)
        {
            ss << "Chatter " << user.userID();
            user.name() = ss.str();
        }
        else
        {
            user.name() = argv[2];
        }
        if(argc == 4)
        {
            numMsg = atoi(argv[3]);
        }
        e.nameServiceWriter << user;

        /** Initialise the chat message */
        Chat::ChatMessage msg;
        msg.userID() = user.userID();
        msg.index() = 0;
        if(user.userID() == TERMINATION_MESSAGE)
        {
            msg.content() = "Termination message.";
        }
        else
        {
            ss.str("");
            ss << "Hi There, I will send you " << numMsg << " more messages.";
            msg.content() = ss.str();
        }

        /**
         * Register a dds::core::InstanceHandle for the message, this causes resources
         * to be preallocated for it
         */
        dds::core::InstanceHandle userHandle = e.chatMessageWriter.register_instance(msg);

        /** Write the message */
        e.chatMessageWriter.write(msg, userHandle);
        std::cout << "Writing message: \"" << msg.content() << "\"" << std::endl;

        /** Wait to ensure messages are sent */
        exampleSleepMilliseconds(1000);

        /** Write remaining messages */
        for(int i = 1; i <= numMsg && user.userID() != TERMINATION_MESSAGE; i++)
        {
            msg.index() = i;
            ss.str("");
            ss << "Message no. " << i;
            msg.content() = ss.str();
            e.chatMessageWriter.write(msg, userHandle);
            std::cout << "Writing message: \"" << msg.content() << "\"" << std::endl;

            /** Wait to ensure messages are sent */
            exampleSleepMilliseconds(1000);
        }

        std::cout << "Completed Chatter example." << std::endl;
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

EXAMPLE_ENTRYPOINT(DCPS_ISOCPP_tutorial_Chatter, examples::dcps::Tutorial::isocpp::Chatter)
