.. _`Appendix B`:


##########
Appendix B
##########

|cpp|

This appendix contains the example, user-written C++ source code 
included with the Vortex OpenSplice Modeler ``Chatroom`` C++ example. 
The Chatroom system is the example used in the
:ref:`Tutorial <Tutorial>`. 

There are two different versions of some of the Chatroom’s C++ modules, 
one version for Linux, the other for Windows, in order to accommodate 
differences between these platforms. 

The source code is given in the following order:

**Chatter Application:**

  `ChatterApplication.cpp, Linux Version`_

  `ChatterApplication.cpp, Windows Version`_

**MessageBoard Application:**

  `MessageBoardApplication.cpp, Linux Version`_

  `MessageBoardApplication.cpp, Windows Version`_

  `ChatMessageDataReaderListenerImpl.h, Linux version`_

  `ChatMessageDataReaderListenerImpl.h, Windows version`_

  `ChatMessageDataReaderListenerImpl.cpp`_

  `NamedMessageDataReaderListenerImpl.h, Linux Version`_

  `NamedMessageDataReaderListenerImpl.h, Windows Version`_

  `NamedMessageDataReaderListenerImpl.cpp`_

**UserLoad Application:**

  `UserLoadApplication.cpp, Linux Version`_

  `UserLoadApplication.cpp, Windows Version`_

  `CheckStatus.h`_

  `CheckStatus.cpp`_


.. _`Appx B CPP`:

.. _`Chatroom Example, C++ Source Code`:

Chatroom Example, C++ Source Code
*********************************



Chatter Application
===================

ChatterApplication.cpp, Linux Version
-------------------------------------
|linux|

.. code-block:: cpp

   ChatterApplication.cpp

   /************************************************************************
    *
    * Copyright (c) 2012 to 2016
    * PrismTech Ltd.
    * All rights Reserved.
    *
    * LOGICAL_NAME:    ChatterApplication.cpp
    * FUNCTION:        Vortex OpenSplice Modeler Tutorial example code.
    * MODULE:          Example for the C++  programming language.
    * DATE             January 2012.
    ************************************************************************
    *
    * This file contains the implementation for the 'ChatterApplication' executable.
    *
    ***/
   #include <string>
   #include <sstream>
   #include <iostream>
   #include <unistd.h>

   #include "ChatterApplication.h"
   #include "CheckStatus.h"

   #define NUM_MSG 10
   #define TERMINATION_MESSAGE -1

   using namespace std;
   using namespace DDS;
   using namespace Chat;

   int main(int argc, char *argv[]) {
      /* Sample definitions */
      ChatMessage *msg; /* Example on Heap */
      NameService ns; /* Example on Stack */

      /* DDS Identifiers */
      InstanceHandle_t userHandle;
      ReturnCode_t status;

      /* Others */
      int ownID = 1;
      char *chatterName= NULL;
      ostringstream buf;

      /* Initialize the application */
      ChatterApplicationWrapperImplementation chatterApplication;

      try {
         chatterApplication.start();
      } catch(WrapperException& e) {
         cout << "Exception occurred while starting the application:" << endl;
         cout << e.what();
         return -1;
      }

      /* Type-specific DDS entities */
      NameServiceDataWriter_ptr nameServer =
            chatterApplication.getParticipantWrapper()->getPublisherWrapper()
               ->getNameServiceDataWriterWrapper()->getDataWriter();
      ChatMessageDataWriter_ptr talker =
            chatterApplication.getParticipantWrapper()->getPublisherWrapper()
               ->getChatMessageDataWriterWrapper()->getDataWriter();

      /* Options: Chatter [ownID [name]] */
      if (argc > 1) {
         istringstream args(argv[1]);
         args >> ownID;
         if (argc > 2) {
            chatterName = argv[2];
         }
      }

      /* Initialize the NameServer attributes */
      ns.userID = ownID;
      if (chatterName) {
         ns.name = string_dup(chatterName);
      } else {
         buf << "Chatter " << ownID;
         ns.name = string_dup(buf.str().c_str() );
      }

      /* Write the user-information into the system 
         (registering the instance implicitly). */
      status = nameServer->write(ns, HANDLE_NIL);
      checkStatus(status, "NameServiceDataWriter::write");

      /* Initialize the chat messages on Heap. */
      msg = new ChatMessage();
      checkHandle(msg, "new ChatMessage");
      msg->userID = ownID;
      msg->index = 0;
      buf.str(string("") );
      if (ownID == TERMINATION_MESSAGE) {
         buf << "Termination message.";
      } else {
         buf << "Hi there, I will send you " << NUM_MSG << " more messages.";
      }
      msg->content = string_dup(buf.str().c_str() );
      cout << "Writing message: \"" << msg->content << "\"" << endl;

      /* Register a chat message for this user (pre-allocating resources for it!!) */
      userHandle = talker->register_instance(*msg);

      /* Write a message using the pre-generated instance handle. */
      status = talker->write(*msg, userHandle);
      checkStatus(status, "ChatMessageDataWriter::write");

      sleep(1); /* do not run so fast! */

      /* Write any number of messages, 
         re-using the existing string-buffer: no leak!!. */
      for (int i = 1; i <= NUM_MSG && ownID != TERMINATION_MESSAGE; i++) {
         buf.str(string("") );
         msg->index = i;
         buf << "Message no. " << i;
         msg->content = string_dup(buf.str().c_str() );
         cout << "Writing message: \"" << msg->content << "\"" << endl;
         status = talker->write(*msg, userHandle);
         checkStatus(status, "ChatMessageDataWriter::write");
         sleep(1); /* do not run so fast! */
      }

      /* Leave the room by disposing and unregistering the message instance. */
      status = talker->dispose(*msg, userHandle);
      checkStatus(status, "ChatMessageDataWriter::dispose");
      status = talker->unregister_instance(*msg, userHandle);
      checkStatus(status, "ChatMessageDataWriter::unregister_instance");

      /* Also unregister our name. */
      status = nameServer->unregister_instance(ns, HANDLE_NIL);
      checkStatus(status, "NameServiceDataWriter::unregister_instance");

      /* Release the data-samples. */
      delete msg; // msg allocated on heap: explicit de-allocation required!!

      /* stop application */
      try {
         chatterApplication.stop();
      } catch(WrapperException& e) {
         cout << "Exception occurred while stopping the application:" << endl;
         cout << e.what();
         return -1;
      }

      return 0;
   }




ChatterApplication.cpp, Windows Version
---------------------------------------
|windows|

.. code-block:: cpp

   ChatterApplication.cpp

   /************************************************************************
    *
    * Copyright (c) 2012 to 2016
    * PrismTech Ltd.
    * All rights Reserved.
    *
    * LOGICAL_NAME:    ChatterApplication.cpp
    * FUNCTION:        Vortex OpenSplice Modeler Tutorial example code.
    * MODULE:          Example for the C++  programming language.
    * DATE             January 2012.
    ************************************************************************
    *
    * This file contains the implementation for the 'ChatterApplication' executable.
    *
    ***/
   #include <string>
   #include <sstream>
   #include <iostream>

   #include "ChatterApplication.h"
   #include "CheckStatus.h"

   #define NUM_MSG 10
   #define TERMINATION_MESSAGE -1

   using namespace std;
   using namespace DDS;
   using namespace Chat;

   int main(int argc, char *argv[]) {
      /* Sample definitions */
      ChatMessage *msg; /* Example on Heap */
      NameService ns; /* Example on Stack */

      /* DDS Identifiers */
      InstanceHandle_t userHandle;
      ReturnCode_t status;

      /* Others */
      int ownID = 1;
      char *chatterName= NULL;
      ostringstream buf;

      /* Initialize the application */
      ChatterApplicationWrapperImplementation chatterApplication;

      try {
         chatterApplication.start();
      } catch(WrapperException& e) {
         cout << "Exception occurred while starting the application:" << endl;
         cout << e.what();
         return -1;
      }

      /* Type-specific DDS entities */
      NameServiceDataWriter_ptr nameServer =
            chatterApplication.getParticipantWrapper()->getPublisherWrapper()
            ->getNameServiceDataWriterWrapper()->getDataWriter();
      ChatMessageDataWriter_ptr talker =
            chatterApplication.getParticipantWrapper()->getPublisherWrapper()
            ->getChatMessageDataWriterWrapper()->getDataWriter();

      /* Options: Chatter [ownID [name]] */
      if (argc > 1) {
         istringstream args(argv[1]);
         args >> ownID;
         if (argc > 2) {
            chatterName = argv[2];
         }
      }

      /* Initialize the NameServer attributes */
      ns.userID = ownID;
      if (chatterName) {
         ns.name = string_dup(chatterName);
      } else {
         buf << "Chatter " << ownID;
         ns.name = string_dup(buf.str().c_str() );
      }

      /* Write the user-information into the system 
         (registering the instance implicitly). */
      status = nameServer->write(ns, HANDLE_NIL);
      checkStatus(status, "NameServiceDataWriter::write");

      /* Initialize the chat messages on Heap. */
      msg = new ChatMessage();
      checkHandle(msg, "new ChatMessage");
      msg->userID = ownID;
      msg->index = 0;
      buf.str(string("") );
      if (ownID == TERMINATION_MESSAGE) {
         buf << "Termination message.";
      } else {
         buf << "Hi there, I will send you " << NUM_MSG << " more messages.";
      }
      msg->content = string_dup(buf.str().c_str() );
      cout << "Writing message: \"" << msg->content << "\"" << endl;

      /* Register a chat message for this user (pre-allocating resources for it!!) */
      userHandle = talker->register_instance(*msg);

      /* Write a message using the pre-generated instance handle. */
      status = talker->write(*msg, userHandle);
      checkStatus(status, "ChatMessageDataWriter::write");

      sleep(1); /* do not run so fast! */

      /* Write any number of messages, 
         re-using the existing string-buffer: no leak!!. */
      for (int i = 1; i <= NUM_MSG && ownID != TERMINATION_MESSAGE; i++) {
         buf.str(string("") );
         msg->index = i;
         buf << "Message no. " << i;
         msg->content = string_dup(buf.str().c_str() );
         cout << "Writing message: \"" << msg->content << "\"" << endl;
         status = talker->write(*msg, userHandle);
         checkStatus(status, "ChatMessageDataWriter::write");
         Sleep(1000); /* do not run so fast! */
      }

      /* Leave the room by disposing and unregistering the message instance. */
      status = talker->dispose(*msg, userHandle);
      checkStatus(status, "ChatMessageDataWriter::dispose");
      status = talker->unregister_instance(*msg, userHandle);
      checkStatus(status, "ChatMessageDataWriter::unregister_instance");

      /* Also unregister our name. */
      status = nameServer->unregister_instance(ns, HANDLE_NIL);
      checkStatus(status, "NameServiceDataWriter::unregister_instance");

      /* Release the data-samples. */
      delete msg; // msg allocated on heap: explicit de-allocation required!!

      /* stop application */
      try {
         chatterApplication.stop();
      } catch(WrapperException& e) {
         cout << "Exception occurred while stopping the application:" << endl;
         cout << e.what();
         return -1;
      }

      return 0;
   }


MessageBoard Application
========================

MessageBoardApplication.cpp, Linux Version
------------------------------------------
|linux|

.. code-block:: cpp

   MessageBoardApplication.cpp

   /************************************************************************
    *
    * Copyright (c) 2012 to 2016
    * PrismTech Ltd.
    * All rights Reserved.
    *
    * LOGICAL_NAME:    MessageBoardApplication.cpp
    * FUNCTION:        Vortex OpenSplice Modeler Tutorial example code.
    * MODULE:          Tutorial for the C++ programming language.
    * DATE             January 2012.
    ************************************************************************
    *
    * This file contains the implementation for the 
    * 'MessageBoardApplication' executable.
    *
    ***/

   #include <iostream>
   #include <string>
   #include <sstream>
   #include <unistd.h>

   #include "CheckStatus.h"
   #include "MessageBoardApplication.h"

   #include "ChatMessageDataReaderListenerImpl.h"
   #include "NamedMessageDataReaderListenerImpl.h"

   using namespace std;
   using namespace DDS;
   using namespace Chat;

   #define TERMINATION_MESSAGE -1

   int main(int argc, char *argv[]) {
      /* DDS Identifiers */
      string ownID = "0";

      /* Options: MessageBoard [ownID] */
      /* Messages having owner ownID will be ignored */
      if (argc > 1) {
         istringstream args(argv[1]);
         args >> ownID;;
      }

      MessageBoardApplicationWrapperImplementation messageBoardApplication;

      try {
         StringSeq exprParams;
         exprParams.length(1);
         exprParams[0] = DDS::string_dup(ownID.c_str());
         messageBoardApplication.getParticipantWrapper()
            ->getNamedMessageFilteredTopicWrapper()->setExpressionParameters(
            exprParams);
         /* Initialize the application */
         messageBoardApplication.start();
      } catch (WrapperException& e) {
         cout << "Exception occurred while initializing the application:"
               << endl;
         cout << e.what();
         return -1;
      }

      /* Create the listeners for the MessageBoard application */
      ChatMessageDataReaderListenerImpl* chatMessageDataReaderListener =
            new ChatMessageDataReaderListenerImpl(&messageBoardApplication);

      /* put the object in a smart pointer for resource management */
      DDS::DataReaderListener_var chatMessageDataReaderListenerVar(
            chatMessageDataReaderListener);

      NamedMessageDataReaderListenerImpl* namedMessageDataReaderListener =
            new NamedMessageDataReaderListenerImpl(&messageBoardApplication);

      DDS::DataReaderListener_var namedMessageDataReaderListenerVar(
            namedMessageDataReaderListener);

      try {
         /* Attach the ChatMessageDataReaderListener to the ChatMessageDataReader */
         messageBoardApplication.getPrivateParticipantWrapper()
            ->getSubscriberWrapper()->getChatMessageDataReaderWrapper()->attach(
               chatMessageDataReaderListener);

         /* Attach the NamedMessageDataReaderListener to NamedMessageDataReader */
         messageBoardApplication.getParticipantWrapper()->getSubscriberWrapper()
            ->getNamedMessageDataReaderWrapper()->attach(
               namedMessageDataReaderListener);
      } catch (WrapperException& e) {
         cout << "Exception occurred while attaching a listener:" << endl;
         cout << e.what();
         return -1;
      }

      cout
         << "MessageBoard has opened: send ChatMessage with userID = -1 to close it."
         << endl << endl;

      /* Wait for the ChatMessageDataReaderListener to finish */
      while (!chatMessageDataReaderListener->isTerminated()) {
         sleep(1);
      }

      /* Wait for the NamedMessageDataReaderListener to finish */
      while (!namedMessageDataReaderListener->isTerminated()) {
         sleep(1);
      }

      cout << "Termination message received: exiting..." << endl;

      try {
         /* Detach the ChatMessageDataReaderListener to the ChatMessageDataReader */
         messageBoardApplication.getPrivateParticipantWrapper()
            ->getSubscriberWrapper()->getChatMessageDataReaderWrapper()->detach(
               chatMessageDataReaderListener);

         /* Detach the NamedMessageDataReaderListener to the NamedMessageDataReader */
         messageBoardApplication.getParticipantWrapper()->getSubscriberWrapper()
            ->getNamedMessageDataReaderWrapper()->detach(
               namedMessageDataReaderListener);
      } catch (WrapperException& e) {
         cout << "Exception occurred while detaching a listener:" << endl;
         cout << e.what();
         return -1;
      }

      chatMessageDataReaderListener->cleanup();

      try {
         /* Stop the application */
         messageBoardApplication.stop();
      } catch (WrapperException& e) {
         cout << "Exception occurred while stopping the application:" << endl;
         cout << e.what();
         return -1;
      }

      return 0;
   }


MessageBoardApplication.cpp, Windows Version
--------------------------------------------
|windows|

.. code-block:: cpp

   MessageBoardApplication.cpp

   /************************************************************************
    *
    * Copyright (c) 2012 to 2016
    * PrismTech Ltd.
    * All rights Reserved.
    *
    * LOGICAL_NAME:    MessageBoardApplication.cpp
    * FUNCTION:        Vortex OpenSplice Modeler Tutorial example code.
    * MODULE:          Tutorial for the C++ programming language.
    * DATE             January 2012.
    ************************************************************************
    *
    * This file contains the implementation for the 'MessageBoard' executable.
    *
    ***/

   #include <iostream>
   #include <string>
   #include <sstream>

   #include "ccpp_dds_dcps.h"
   #include "CheckStatus.h"
   #include "exported_MessageBoardApplicationDcps.h"
   #include "Chat/MessageBoardApplicationWrapperImplementation.h"
   #include "Chat/NamedMessageFilteredTopicWrapper.h"

   #include "Chat/MessageBoardApplication/ParticipantWrapper.h"
   #include "Chat/MessageBoardApplication/Participant/SubscriberWrapper.h"
   #include "Chat/MessageBoardApplication/Participant/Subscriber/NamedMessageDataReaderWrapper.h"

   #include "Chat/MessageBoardApplication/PrivateParticipantWrapper.h"
   #include "Chat/MessageBoardApplication/PrivateParticipant/SubscriberWrapper.h"
   #include "Chat/MessageBoardApplication/PrivateParticipant/Subscriber/ChatMessageDataReaderWrapper.h"

   #include "ChatMessageDataReaderListenerImpl.h"
   #include "NamedMessageDataReaderListenerImpl.h"

   using namespace std;
   using namespace DDS;
   using namespace Chat;

   #define TERMINATION_MESSAGE -1

   int main(int argc, char *argv[])
   {
      /* DDS Identifiers */
      string ownID = "0";

      /* Options: MessageBoard [ownID] */
      /* Messages having owner ownID will be ignored */
      if (argc > 1)
      {
         istringstream args(argv[1]);
         args >> ownID;;
      }

      MessageBoardApplicationWrapperImplementation messageBoardApplication;

      StringSeq exprParams;
      exprParams.length(1);
      exprParams[0] = DDS::string_dup(ownID.c_str());

      messageBoardApplication.getParticipantWrapper()
         ->getNamedMessageFilteredTopicWrapper()->set_expression_parameters(
            exprParams);

      /* Initialize the application */
      messageBoardApplication.start();

      /* Create the listeners for the MessageBoard application */
      ChatMessageDataReaderListenerImpl* chatMessageDataReaderListener =
            new ChatMessageDataReaderListenerImpl(&messageBoardApplication);

      /* put the object in a smart pointer for resource management */
      DDS::DataReaderListener_var
            chatMessageDataReaderListenerVar(chatMessageDataReaderListener);

      NamedMessageDataReaderListenerImpl* namedMessageDataReaderListener =
            new NamedMessageDataReaderListenerImpl(&messageBoardApplication);

      DDS::DataReaderListener_var
            namedMessageDataReaderListenerVar(namedMessageDataReaderListener);

      /* Attach the ChatMessageDataReaderListener to the ChatMessageDataReader */
      ReturnCode_t status = messageBoardApplication.getPrivateParticipantWrapper()
         ->getSubscriberWrapper()->getChatMessageDataReaderWrapper()
         ->set_listener(chatMessageDataReaderListener,
            chatMessageDataReaderListener->getStatusMask());
      checkStatus(status, "Chat::ChatMessageDataReader::set_listener");

      /* Attach the NamedMessageDataReaderListener to the NamedMessageDataReader */
      status = messageBoardApplication.getParticipantWrapper()
         ->getSubscriberWrapper()->getNamedMessageDataReaderWrapper()->getDataReader()
         ->set_listener(namedMessageDataReaderListener,
            namedMessageDataReaderListener->getStatusMask());
      checkStatus(status, "Chat::ChatMessageDataReader::set_listener");

      cout
         << "MessageBoard has opened: send ChatMessage with userID = -1 to close it."
         << endl << endl;

      /* Wait for the ChatMessageDataReaderListener to finish */
      while (!chatMessageDataReaderListener->isTerminated())
      {
         Sleep(1000);
      }

      /* Wait for the NamedMessageDataReaderListener to finish */
      while (!namedMessageDataReaderListener->isTerminated())
      {
         Sleep(1000);
      }

      cout << "Termination message received: exiting..." << endl;

      /* Detach the ChatMessageDataReaderListener to the ChatMessageDataReader */
      messageBoardApplication.getPrivateParticipantWrapper()->getSubscriberWrapper()
         ->set_listener(0, 0);

      /* Detach the NamedMessageDataReaderListener to the NamedMessageDataReader */
      messageBoardApplication.getParticipantWrapper()->getSubscriberWrapper()
         ->set_listener(0, 0);

      chatMessageDataReaderListener->cleanup();

      /* Stop the application */
      messageBoardApplication.stop();

      return 0;
   }


ChatMessageDataReaderListenerImpl.h, Linux version
--------------------------------------------------
|linux|

.. code-block:: cpp
  
   ChatMessageDataReaderListenerImpl.h

   /************************************************************************
    *
    * Copyright (c) 2012 to 2016
    * PrismTech Ltd.
    * All rights Reserved.
    *
    * LOGICAL_NAME:    ChatMessageDataReaderListenerImpl.h
    * FUNCTION:        Vortex OpenSplice Modeler Tutorial example code.
    * MODULE:          Tutorial for the C++ programming language.
    * DATE             January 2012.
    ************************************************************************
    *
    * This file contains the implementation for the 'MessageBoard' executable.
    *
    ***/
   #ifndef _CHATMESSAGEDATAREADERLISTENERIMPL_H_
   #define _CHATMESSAGEDATAREADERLISTENERIMPL_H_

   #include <string>

   #include <pthread.h>

   #include "Chat/MessageBoardApplication/ChatMessageDataReaderListener.h"
   #include "Chat/MessageBoardApplicationWrapper.h"
   #include "exported_MessageBoardApplicationDcps.h"

   class ChatMessageDataReaderListenerImpl :
      public Chat::MessageBoardApplication::ChatMessageDataReaderListener
   {
   public:
      ChatMessageDataReaderListenerImpl(
            const Chat::MessageBoardApplicationWrapper* messageBoardApplication);

      void on_data_available(DDS::DataReader_ptr dataReader);
      void cleanup();
      bool isTerminated();

   private:

      class IsTerminated
      {
      public:
         IsTerminated()
         {
            m_isTerminated = false;
            pthread_mutex_init(&m_mutex, 0);
         }

         virtual ~IsTerminated()
         {
            pthread_mutex_destroy(&m_mutex);
         }

         bool isTerminated()
         {
            bool ret;

            pthread_mutex_lock(&m_mutex);
            ret = m_isTerminated;
            pthread_mutex_unlock(&m_mutex);

            return ret;
         }

         void setTerminated(bool isTerminated)
         {
            pthread_mutex_lock(&m_mutex);
            m_isTerminated = isTerminated;
            pthread_mutex_unlock(&m_mutex);
         }

      private:
         pthread_mutex_t m_mutex;
         bool m_isTerminated;
      };

      IsTerminated m_isTerminated;

      static int TERMINATION_MESSAGE;

      const Chat::MessageBoardApplicationWrapper* m_messageBoardApplication;
      Chat::ChatMessageDataReader_ptr m_chatMsgReader;
      Chat::NameServiceDataReader_ptr m_nameServiceReader;
      Chat::NamedMessageDataWriter_ptr m_namedMessageWriter;

      DDS::QueryCondition_ptr m_nameFinder;
      DDS::StringSeq m_nameFinderParams;

      DDS::ReadCondition_ptr m_newMessages;

      int m_previousID;
      DDS::String_mgr m_userName;

   };

   #endif // _CHATMESSAGEDATAREADERLISTENERIMPL_H_  


ChatMessageDataReaderListenerImpl.h, Windows version
----------------------------------------------------
|windows|

.. code-block:: cpp

   ChatMessageDataReaderListenerImpl.h

   /************************************************************************
    *
    * Copyright (c) 2012 to 2016
    * PrismTech Ltd.
    * All rights Reserved.
    *
    * LOGICAL_NAME:    ChatMessageDataReaderListenerImpl.h
    * FUNCTION:        Vortex OpenSplice Modeler Tutorial example code.
    * MODULE:          Tutorial for the C++ programming language.
    * DATE             January 2012.
    ************************************************************************
    *
    * This file contains the implementation for the 'MessageBoard' executable.
    *
    ***/
   #ifndef _CHATMESSAGEDATAREADERLISTENERIMPL_H_
   #define _CHATMESSAGEDATAREADERLISTENERIMPL_H_

   #include <string>
   #include "Chat/MessageBoardApplication/ChatMessageDataReaderListener.h"
   #include "Chat/MessageBoardApplicationWrapper.h"
   #include "exported_MessageBoardApplicationDcps.h"

   class ChatMessageDataReaderListenerImpl :
      public Chat::MessageBoardApplication::ChatMessageDataReaderListener
   {
   public:
      ChatMessageDataReaderListenerImpl(
            const Chat::MessageBoardApplicationWrapper* messageBoardApplication);

      void on_data_available(DDS::DataReader_ptr dataReader);
      void cleanup();
      bool isTerminated();

   private:

      class IsTerminated
      {
      public:
         IsTerminated()
         {
            m_isTerminated = false;
            m_mutex = CreateMutex(NULL, FALSE, NULL);
         }

         virtual ~IsTerminated()
         {
            CloseHandle(m_mutex);
         }

         bool isTerminated()
         {
            bool ret;

            WaitForSingleObject(m_mutex, INFINITE);
            ret = m_isTerminated;
            ReleaseMutex(m_mutex);

            return ret;
         }

         void setTerminated(bool isTerminated)
         {
            WaitForSingleObject(m_mutex, INFINITE);
            m_isTerminated = isTerminated;
            ReleaseMutex(m_mutex);
         }

      private:
         HANDLE m_mutex;
         bool m_isTerminated;
      };

      IsTerminated m_isTerminated;

      static int TERMINATION_MESSAGE;

      const Chat::MessageBoardApplicationWrapper* m_messageBoardApplication;
      Chat::ChatMessageDataReader_ptr m_chatMsgReader;
      Chat::NameServiceDataReader_ptr m_nameServiceReader;
      Chat::NamedMessageDataWriter_ptr m_namedMessageWriter;

      DDS::QueryCondition_ptr m_nameFinder;
      DDS::StringSeq m_nameFinderParams;

      DDS::ReadCondition_ptr m_newMessages;

      int m_previousID;
      DDS::String_mgr m_userName;

   };

   #endif // _CHATMESSAGEDATAREADERLISTENERIMPL_H_



ChatMessageDataReaderListenerImpl.cpp
-------------------------------------

.. code-block:: cpp

   ChatMessageDataReaderListenerImpl.cpp

   /************************************************************************
    *
    * Copyright (c) 2012 to 2016
    * PrismTech Ltd.
    * All rights Reserved.
    *
    * LOGICAL_NAME:    ChatMessageDataReaderListenerImpl.cpp
    * FUNCTION:        Vortex OpenSplice Modeler Tutorial example code.
    * MODULE:          Tutorial for the C++ programming language.
    * DATE             January 2012.
    ************************************************************************
    *
    * This file contains the implementation for the 'MessageBoard' executable.
    *
    ***/
   #include <sstream>

   #include "ChatMessageDataReaderListenerImpl.h"

   #include "ccpp_dds_dcps.h"
   #include "Chat/MessageBoardApplication/PrivateParticipantWrapper.h"
   #include "Chat/MessageBoardApplication/PrivateParticipant/SubscriberWrapper.h"
   #include "Chat/MessageBoardApplication/PrivateParticipant/Subscriber/ChatMessageDataReaderWrapper.h"
   #include "Chat/MessageBoardApplication/PrivateParticipant/Subscriber/NameServiceDataReaderWrapper.h"
   #include "Chat/MessageBoardApplication/PrivateParticipant/PublisherWrapper.h"
   #include "Chat/MessageBoardApplication/PrivateParticipant/Publisher/NamedMessageDataWriterWrapper.h"
   #include "CheckStatus.h"

   int ChatMessageDataReaderListenerImpl::TERMINATION_MESSAGE = -1;

   using namespace std;
   using namespace DDS;
   using namespace Chat;

   ChatMessageDataReaderListenerImpl::ChatMessageDataReaderListenerImpl(
         const Chat::MessageBoardApplicationWrapper* messageBoardApplication) :
            m_messageBoardApplication(messageBoardApplication),
            m_chatMsgReader(
                  messageBoardApplication->getPrivateParticipantWrapper()
                  ->getSubscriberWrapper()->getChatMessageDataReaderWrapper()
                  ->getDataReader()),
                  m_nameServiceReader(
                  messageBoardApplication->getPrivateParticipantWrapper()
                  ->getSubscriberWrapper()->getNameServiceDataReaderWrapper()
                  ->getDataReader()),
                  m_namedMessageWriter(
                  messageBoardApplication->getPrivateParticipantWrapper()
                  ->getPublisherWrapper()->getNamedMessageDataWriterWrapper()
                  ->getDataWriter()),
                  m_previousID(-1) {
      /* Create a QueryCondition that will look up userName for a specified userID */
      m_nameFinderParams.length(1);
      m_nameFinderParams[0] = string_dup("0");

      m_nameFinder = m_nameServiceReader->create_querycondition(ANY_SAMPLE_STATE,
            ANY_VIEW_STATE, ANY_INSTANCE_STATE, "userID = %0",
            m_nameFinderParams);
      checkHandle(m_nameFinder,
            "Chat::NameServiceDataReader::create_querycondition");

      m_newMessages = m_chatMsgReader->create_readcondition(ANY_SAMPLE_STATE,
            ANY_VIEW_STATE, ANY_INSTANCE_STATE);
      checkHandle(m_newMessages,
            "Chat::ChatMessageDataReader::create_readcondition");

   }

   void ChatMessageDataReaderListenerImpl::on_data_available(
         DDS::DataReader_ptr dataReader) {

      /* Ignore new data if termination message already received */
      if (m_isTerminated.isTerminated()) {
         return;
      }

      bool terminationReceived = false;
      int status;

      if (dataReader == m_chatMsgReader) {
         ChatMessageSeq chatMsgSeq;
         SampleInfoSeq chatMsgInfoSeq;

         status = m_chatMsgReader->take_w_condition(chatMsgSeq, chatMsgInfoSeq,
               LENGTH_UNLIMITED, m_newMessages);
         checkStatus(status, "Chat::ChatMessageDataReader::take_w_condition");

         /* For each message, extract the key-field and find the corresponding name */
         for (unsigned int i = 0; i < chatMsgSeq.length(); i++) {
            NameServiceSeq nameServiceSeq;
            SampleInfoSeq nameServiceInfoSeq;

            /* Set program termination flag if termination message is received */
            if (chatMsgSeq[i].userID == TERMINATION_MESSAGE) {
               terminationReceived = true;
               break;
            }

            /* Find the corresponding named message */
            if (chatMsgSeq[i].userID != m_previousID) {
               m_previousID = chatMsgSeq[i].userID;

               ostringstream previousID;
               previousID << m_previousID;
               m_nameFinderParams[0] = string_dup(previousID.str().c_str());

               status = m_nameFinder->set_query_parameters(m_nameFinderParams);
               checkStatus(status,
                     "QueryCondition::set_query_arguments(m_nameFinderParams)");

               status = m_nameServiceReader->read_w_condition(nameServiceSeq,
                     nameServiceInfoSeq, LENGTH_UNLIMITED, m_nameFinder);
               checkStatus(status,
                     "Chat::NameServiceDataReader::read_w_condition");

               if (status == RETCODE_NO_DATA) {
                  ostringstream os;
                  os << "Name not found!! id = " + m_previousID;
                  m_userName = string_dup(os.str().c_str());
               } else {
                  m_userName = nameServiceSeq[0].name;
               }

               /* Release the name sample again */
               status = m_nameServiceReader->return_loan(nameServiceSeq,
                     nameServiceInfoSeq);
               checkStatus(status, "Chat::NameServiceDataReader::return_loan");
            }

            NamedMessage namedMsg;

            /* Write merged Topic with userName instead of userID */

            namedMsg.userName = m_userName;
            namedMsg.userID = m_previousID;
            namedMsg.index = chatMsgSeq[i].index;
            namedMsg.content = chatMsgSeq[i].content;

            if (chatMsgInfoSeq[i].valid_data) {
               status = m_namedMessageWriter->write(namedMsg, HANDLE_NIL);
               checkStatus(status, "Chat::NamedMessageDataWriter::write");
            }
         }

         status = m_chatMsgReader->return_loan(chatMsgSeq, chatMsgInfoSeq);
         checkStatus(status, "Chat::ChatMessageDataReader::return_loan");

         if (terminationReceived) {
            m_isTerminated.setTerminated(true);
         }
      }

   }

   bool ChatMessageDataReaderListenerImpl::isTerminated() {
      return m_isTerminated.isTerminated();
   }

   void ChatMessageDataReaderListenerImpl::cleanup() {
      /* Remove all Read Conditions from the DataReaders */

      int status = m_nameServiceReader->delete_readcondition(m_nameFinder);
      checkStatus(status,
            "Chat::NameServiceDataReader::delete_readcondition(nameFinder)");

      status = m_chatMsgReader->delete_readcondition(m_newMessages);
      checkStatus(status,
            "Chat::ChatMessageDataReader::delete_readcondition(newMessages)");

   }


NamedMessageDataReaderListenerImpl.h, Linux Version
---------------------------------------------------
|linux|

.. code-block:: cpp

   NamedMessageDataReaderListenerImpl.h

   /************************************************************************
    *
    * Copyright (c) 2012 to 2016
    * PrismTech Ltd.
    * All rights Reserved.
    *
    * LOGICAL_NAME:    NamedMessageDataReaderListenerImpl.h
    * FUNCTION:        Vortex OpenSplice Modeler Tutorial example code.
    * MODULE:          Tutorial for the C++ programming language.
    * DATE             January 2012.
    ************************************************************************
    *
    * This file contains the implementation for the 'MessageBoard' executable.
    *
    ***/
   #ifndef _NAMEDMESSAGEDATAREADERLISTENERIMPL_H_
   #define _NAMEDMESSAGEDATAREADERLISTENERIMPL_H_

   #include <pthread.h>

   #include "exported_MessageBoardApplicationDcps.h"
   #include "Chat/MessageBoardApplicationWrapper.h"
   #include "Chat/MessageBoardApplication/NamedMessageDataReaderListener.h"

   class NamedMessageDataReaderListenerImpl : public Chat::MessageBoardApplication::NamedMessageDataReaderListener
   {
   public:
      NamedMessageDataReaderListenerImpl(const Chat::MessageBoardApplicationWrapper* messageBoardApplication);
      void on_data_available (DDS::DataReader_ptr dataReader);
      bool isTerminated();

   private:
      class IsTerminated
      {
      public:
         IsTerminated()
         {
            m_isTerminated = true;
            pthread_mutex_init(&m_mutex, 0);
         }

         virtual ~IsTerminated()
         {
            pthread_mutex_destroy(&m_mutex);
         }

         bool isTerminated()
         {
            bool ret;

            pthread_mutex_lock(&m_mutex);
            ret = m_isTerminated;
            pthread_mutex_unlock(&m_mutex);

            return ret;
         }

         void setTerminated(bool isTerminated)
         {
            pthread_mutex_lock(&m_mutex);
            m_isTerminated = isTerminated;
            pthread_mutex_unlock(&m_mutex);
         }

      private:
         pthread_mutex_t m_mutex;
         bool m_isTerminated;
      };

      IsTerminated m_isTerminated;

      const Chat::MessageBoardApplicationWrapper* m_messageBoardApplication;

      Chat::NamedMessageDataReader_ptr m_namedMsgReader;
   };

   #endif // _NAMEDMESSAGEDATAREADERLISTENERIMPL_H_


NamedMessageDataReaderListenerImpl.h, Windows Version
-----------------------------------------------------
|windows|

.. code-block:: cpp

   NamedMessageDataReaderListenerImpl.h

   /************************************************************************
    *
    * Copyright (c) 2012 to 2016
    * PrismTech Ltd.
    * All rights Reserved.
    *
    * LOGICAL_NAME:    NamedMessageDataReaderListenerImpl.h
    * FUNCTION:        Vortex OpenSplice Modeler Tutorial example code.
    * MODULE:          Tutorial for the C++ programming language.
    * DATE             January 2012.
    ************************************************************************
    *
    * This file contains the implementation for the 'MessageBoard' executable.
    *
    ***/
   #ifndef _NAMEDMESSAGEDATAREADERLISTENERIMPL_H_
   #define _NAMEDMESSAGEDATAREADERLISTENERIMPL_H_

   #include <Windows.h>

   #include "exported_MessageBoardApplicationDcps.h"
   #include "Chat/MessageBoardApplicationWrapper.h"
   #include "Chat/MessageBoardApplication/NamedMessageDataReaderListener.h"

   class NamedMessageDataReaderListenerImpl : public Chat::MessageBoardApplication::NamedMessageDataReaderListener
   {
   public:
      NamedMessageDataReaderListenerImpl(const Chat::MessageBoardApplicationWrapper* messageBoardApplication);
      void on_data_available (DDS::DataReader_ptr dataReader);
      bool isTerminated();

   private:
         class IsTerminated
      {
      public:
         IsTerminated()
         {
            m_isTerminated = false;
            m_mutex = CreateMutex(NULL, FALSE, NULL);
         }

         virtual ~IsTerminated()
         {
            CloseHandle(m_mutex);
         }

         bool isTerminated()
         {
            bool ret;

            WaitForSingleObject(m_mutex, INFINITE);
            ret = m_isTerminated;
            ReleaseMutex(m_mutex);

            return ret;
         }

         void setTerminated(bool isTerminated)
         {
            WaitForSingleObject(m_mutex, INFINITE);
            m_isTerminated = isTerminated;
            ReleaseMutex(m_mutex);
         }

      private:
         HANDLE m_mutex;
         bool m_isTerminated;
      };

      IsTerminated m_isTerminated;

      const Chat::MessageBoardApplicationWrapper* m_messageBoardApplication;

      Chat::NamedMessageDataReader_ptr m_namedMsgReader;
   };

   #endif // _NAMEDMESSAGEDATAREADERLISTENERIMPL_H_


NamedMessageDataReaderListenerImpl.cpp
--------------------------------------

.. code-block:: cpp

   NamedMessageDataReaderListenerImpl.cpp

   /************************************************************************
    *
    * Copyright (c) 2012 to 2016
    * PrismTech Ltd.
    * All rights Reserved.
    *
    * LOGICAL_NAME:    NamedMessageDataReaderListenerImpl.cpp
    * FUNCTION:        Vortex OpenSplice Modeler Tutorial example code.
    * MODULE:          Tutorial for the C++ programming language.
    * DATE             January 2012.
    ************************************************************************
    *
    * This file contains the implementation for the 'MessageBoard' executable.
    *
    ***/
   #include <iostream>

   #include "NamedMessageDataReaderListenerImpl.h"

   #include "Chat/MessageBoardApplication/ParticipantWrapper.h"
   #include "Chat/MessageBoardApplication/Participant/SubscriberWrapper.h"
   #include "Chat/MessageBoardApplication/Participant/Subscriber/NamedMessageDataReaderWrapper.h"

   #include "CheckStatus.h"

   using namespace DDS;
   using namespace Chat;
   using namespace std;

   NamedMessageDataReaderListenerImpl::NamedMessageDataReaderListenerImpl(
         const Chat::MessageBoardApplicationWrapper* messageBoardApplication) :
            m_messageBoardApplication(messageBoardApplication),
            m_namedMsgReader(messageBoardApplication->getParticipantWrapper()->getSubscriberWrapper()->getNamedMessageDataReaderWrapper()->getDataReader())
   {
      m_isTerminated.setTerminated(true);
   }

   void NamedMessageDataReaderListenerImpl::on_data_available(
         DDS::DataReader_ptr dataReader)
   {
      NamedMessageSeq namedMsgSeq;
      DDS::SampleInfoSeq infoSeq;

      m_isTerminated.setTerminated(false);

      int status = m_namedMsgReader->take(namedMsgSeq, infoSeq, LENGTH_UNLIMITED,
            NOT_READ_SAMPLE_STATE, ANY_VIEW_STATE, ALIVE_INSTANCE_STATE);
      checkStatus(status, "Chat::NamedMessageDataReader::read");

      /* For each message, print the message */
      for (unsigned int i = 0; i < namedMsgSeq.length(); i++)
      {
         cout << namedMsgSeq[i].userName << ": " << namedMsgSeq[i].content << endl;
      }

      status = m_namedMsgReader->return_loan(namedMsgSeq, infoSeq);
      checkStatus(status, "Chat::NamedMessageDataReader::return_loan");

      m_isTerminated.setTerminated(true);
   }

   bool NamedMessageDataReaderListenerImpl::isTerminated()
   {
      return m_isTerminated.isTerminated();
   }



UserLoad Application
====================

UserLoadApplication.cpp, Linux Version
--------------------------------------
|linux|

.. code-block:: cpp

   UserLoadApplication.cpp

   /************************************************************************
    *
    * Copyright (c) 2012 to 2016
    * PrismTech Ltd.
    * All rights Reserved.
    *
    * LOGICAL_NAME:    UserLoadApplication.cpp
    * FUNCTION:        Vortex OpenSplice Modeler Tutorial example code.
    * MODULE:          Tutorial for the C++ programming language.
    * DATE             January 2012.
    ************************************************************************
    *
    * This file contains the implementation for the 'UserLoadApplication' executable.
    *
    ***/

   #include <iostream>
   #include <sstream>
   #include <unistd.h>
   #include <string.h>
   #include <pthread.h>
   #include <assert.h>

   #include "UserLoadApplication.h"
   #include "CheckStatus.h"

   using namespace std;
   using namespace DDS;
   using namespace Chat;

   /* entities required by all threads. */
   static DDS::GuardCondition_ptr escape;

   /* Sleeper thread: sleeps 60 seconds and then triggers the WaitSet. */
   void * delayedEscape(void *arg) {
      DDS::ReturnCode_t status;

      sleep(60); /* wait for 60 sec. */
      status = escape->set_trigger_value(TRUE);
      checkStatus(status, "DDS::GuardCondition::set_trigger_value");

      return NULL;
   }

   int main(int argc, char *argv[]) {
      /* DDS Identifiers */
      ReturnCode_t status;
      ConditionSeq guardList;

      ChatMessageSeq msgList;
      NameServiceSeq nsList;
      SampleInfoSeq infoSeq;
      SampleInfoSeq infoSeq2;

      /* Others */
      StringSeq args;

      bool closed = false;
      Long prevCount = 0;
      pthread_t tid;

      UserLoadApplicationWrapperImplementation userLoadApplication;

      /* Initialize the Query Arguments. */
      args.length(1);
      args[0UL] = "0";

      try {
         userLoadApplication.getQueryConditionWrapper()->setQueryParameters(args);

         /* Initialize the application */
         userLoadApplication.start();
      } catch(WrapperException& e) {
         cout << "Error while initializing the application:" << endl;
         cout << e.what() << endl;
         return -1;
      }

      try {
         /* start the WaitSet */
         userLoadApplication.getUserLoadWaitSetWrapper()->start();
      } catch(WrapperException& e) {
         cout << "Error while starting the WaitSet:" << endl;
         cout << e.what() << endl;
         userLoadApplication.stop();
         return -1;
      }

      WaitSet_ptr userLoadWS = userLoadApplication.getUserLoadWaitSetWrapper()->getWaitSet();

      /* Generic DDS entities */
      LivelinessChangedStatus         livChangStatus;

      escape = userLoadApplication.getGuardConditionWrapper()->getCondition();

      /* Type-specific DDS entities */
      NameServiceDataReader_ptr nameServer = userLoadApplication.getParticipantWrapper()->getSubscriberWrapper()->getNameServiceDataReaderWrapper()->getDataReader();
      ChatMessageDataReader_ptr loadAdmin = userLoadApplication.getParticipantWrapper()->getSubscriberWrapper()->getChatMessageDataReaderWrapper()->getDataReader();
      QueryCondition_ptr singleUser = userLoadApplication.getQueryConditionWrapper()->getCondition();
       ReadCondition_ptr newUser = userLoadApplication.getReadConditionWrapper()->getCondition();
       StatusCondition_ptr leftUser = userLoadApplication.getStatusConditionWrapper()->getCondition();

       /* Initialize and pre-allocate the GuardList used to obtain the triggered Conditions. */
       guardList.length(3);

       /* Remove all known Users that are not currently active. */
       status = nameServer->take(
           nsList,
           infoSeq,
           LENGTH_UNLIMITED,
           ANY_SAMPLE_STATE,
           ANY_VIEW_STATE,
           NOT_ALIVE_INSTANCE_STATE);
       checkStatus(status, "Chat::NameServiceDataReader::take");
       status = nameServer->return_loan(nsList, infoSeq);
       checkStatus(status, "Chat::NameServiceDataReader::return_loan");

      /* Start the sleeper thread */
      pthread_create (&tid, NULL, delayedEscape, NULL);

      while (!closed) {
         /* Wait until at least one of the Conditions in the waitset triggers. */
         status = userLoadWS->wait(guardList, DURATION_INFINITE);
         checkStatus(status, "DDS::WaitSet::wait");

         /* Walk over all guards to display information */
         for (ULong i = 0; i < guardList.length(); i++) {
            if (guardList[i].in() == newUser ) {
               /* The newUser ReadCondition contains data */
               status = nameServer->read_w_condition(nsList, infoSeq,
                     LENGTH_UNLIMITED, newUser );
               checkStatus(status,
                     "Chat::NameServiceDataReader::read_w_condition");

               for (ULong j = 0; j < nsList.length(); j++) {
                  cout << "New user: " << nsList[j].name << endl;
               }
               status = nameServer->return_loan(nsList, infoSeq);
               checkStatus(status, "Chat::NameServiceDataReader::return_loan");

            } else if (guardList[i].in() == leftUser ) {
               /* Some liveliness has changed (either a DataWriter joined or a DataWriter left) */
               status
                     = loadAdmin->get_liveliness_changed_status(livChangStatus);
               checkStatus(status,
                     "DDS::DataReader::get_liveliness_changed_status");

               if (livChangStatus.alive_count < prevCount) {
                  /* A user has left the ChatRoom, since a DataWriter lost its liveliness */
                  /* Take the effected users so tey will not appear in the list later on. */

                  status = nameServer->take(nsList, infoSeq,
                        LENGTH_UNLIMITED, ANY_SAMPLE_STATE, ANY_VIEW_STATE,
                        NOT_ALIVE_INSTANCE_STATE);
                  checkStatus(status, "Chat::NameServiceDataReader::take");

                  for (ULong j = 0; j < nsList.length(); j++) {
                     /* re-apply query arguments */
                     ostringstream numberString;
                     numberString << nsList[j].userID;
                     args[0UL] = numberString.str().c_str();
                     status = singleUser->set_query_parameters(args);
                     checkStatus(status,
                           "DDS::QueryCondition::set_query_parameters");

                     /* Read this users history */
                     status = loadAdmin->take_w_condition(msgList, infoSeq2,
                           LENGTH_UNLIMITED, singleUser );
                     checkStatus(status,
                           "Chat::ChatMessageDataReader::take_w_condition");

                     /* Display the user and his history */
                     cout << "Departed user " << nsList[j].name
                           << " has sent " << msgList.length()
                           << " messages." << endl;
                     status = loadAdmin->return_loan(msgList, infoSeq2);
                     checkStatus(status,
                           "Chat::ChatMessageDataReader::return_loan");
                  }
                  status = nameServer->return_loan(nsList, infoSeq);
                  checkStatus(status,
                        "Chat::NameServiceDataReader::return_loan");
               }
               prevCount = livChangStatus.alive_count;

            } else if (guardList[i].in() == escape) {
               cout << "UserLoad has terminated." << endl;
               closed = true;
            } else {
               assert(0);
            };
         } /* for */
      } /* while (!closed) */

      try {
         /* Stop the application */
         userLoadApplication.stop ();
      } catch(WrapperException& e) {
         cout << "Error while stopping the application:" << endl;
         cout << e.what() << endl;
         return -1;
      }

      return 0;
   }



UserLoadApplication.cpp, Windows Version
----------------------------------------
|windows|

.. code-block:: cpp

   UserLoadApplication.cpp

   /************************************************************************
    *
    * Copyright (c) 2012 to 2016
    * PrismTech Ltd.
    * All rights Reserved.
    *
    * LOGICAL_NAME:    UserLoadApplication.cpp
    * FUNCTION:        Vortex OpenSplice Modeler Tutorial example code.
    * MODULE:          Tutorial for the C++ programming language.
    * DATE             January 2012.
    ************************************************************************
    *
    * This file contains the implementation for the 'UserLoadApplication' executable.
    *
    ***/

   #include <iostream>
   #include <sstream>
   #include <string.h>
   #include <assert.h>

   #include "UserLoadApplication.h"
   #include "CheckStatus.h"

   using namespace std;
   using namespace DDS;
   using namespace Chat;

   /* entities required by all threads. */
   static DDS::GuardCondition_ptr escape;

   /* Sleeper thread: sleeps 60 seconds and then triggers the WaitSet. */
   DWORD WINAPI
   delayedEscape(
       LPVOID arg)
   {
       DDS::ReturnCode_t status;

       Sleep(60000);     /* wait for 60 sec. */
       status = escape->set_trigger_value(TRUE);
       checkStatus(status, "DDS::GuardCondition::set_trigger_value");

       return 0;
   }

   int main(int argc, char *argv[]) {
      /* DDS Identifiers */
      ReturnCode_t status;
      ConditionSeq guardList;

      ChatMessageSeq msgList;
      NameServiceSeq nsList;
      SampleInfoSeq infoSeq;
      SampleInfoSeq infoSeq2;

      /* Others */
      StringSeq args;

      bool closed = false;
      Long prevCount = 0;
      DWORD tid;
      HANDLE tHandle = INVALID_HANDLE_VALUE;

      UserLoadApplicationWrapperImplementation userLoadApplication;

      /* Initialize the Query Arguments. */
      args.length(1);
      args[0UL] = "0";

      try {
         userLoadApplication.getQueryConditionWrapper()->setQueryParameters(args);

         /* Initialize the application */
         userLoadApplication.start();
      } catch(WrapperException& e) {
         cout << "Error while initializing the application:" << endl;
         cout << e.what() << endl;
         return -1;
      }

      try {
         /* start the WaitSet */
         userLoadApplication.getUserLoadWaitSetWrapper()->start();
      } catch(WrapperException& e) {
         cout << "Error while starting the WaitSet:" << endl;
         cout << e.what() << endl;
         userLoadApplication.stop();
         return -1;
      }

      WaitSet_ptr userLoadWS = userLoadApplication.getUserLoadWaitSetWrapper()->getWaitSet();

      /* Generic DDS entities */
      LivelinessChangedStatus         livChangStatus;

      escape = userLoadApplication.getGuardConditionWrapper()->getCondition();

      /* Type-specific DDS entities */
      NameServiceDataReader_ptr nameServer = userLoadApplication.getParticipantWrapper()->getSubscriberWrapper()->getNameServiceDataReaderWrapper()->getDataReader();
      ChatMessageDataReader_ptr loadAdmin = userLoadApplication.getParticipantWrapper()->getSubscriberWrapper()->getChatMessageDataReaderWrapper()->getDataReader();
      QueryCondition_ptr singleUser = userLoadApplication.getQueryConditionWrapper()->getCondition();
      ReadCondition_ptr newUser = userLoadApplication.getReadConditionWrapper()->getCondition();
      StatusCondition_ptr leftUser = userLoadApplication.getStatusConditionWrapper()->getCondition();

      /* Initialize and pre-allocate the GuardList used to obtain the triggered Conditions. */
      guardList.length(3);

      /* Remove all known Users that are not currently active. */
      status = nameServer->take(
          nsList,
          infoSeq,
          LENGTH_UNLIMITED,
          ANY_SAMPLE_STATE,
          ANY_VIEW_STATE,
          NOT_ALIVE_INSTANCE_STATE);
      checkStatus(status, "Chat::NameServiceDataReader::take");
      status = nameServer->return_loan(nsList, infoSeq);
      checkStatus(status, "Chat::NameServiceDataReader::return_loan");

      /* Start the sleeper thread */
      tHandle = CreateThread(NULL, 0, delayedEscape, NULL, 0, &tid);

      while (!closed) {
         /* Wait until at least one of the Conditions in the waitset triggers. */
         status = userLoadWS->wait(guardList, DURATION_INFINITE);
         checkStatus(status, "DDS::WaitSet::wait");

         /* Walk over all guards to display information */
         for (ULong i = 0; i < guardList.length(); i++) {
            if (guardList[i].in() == newUser ) {
               /* The newUser ReadCondition contains data */
               status = nameServer->read_w_condition(nsList, infoSeq,
                     LENGTH_UNLIMITED, newUser );
               checkStatus(status,
                     "Chat::NameServiceDataReader::read_w_condition");

               for (ULong j = 0; j < nsList.length(); j++) {
                  cout << "New user: " << nsList[j].name << endl;
               }
               status = nameServer->return_loan(nsList, infoSeq);
               checkStatus(status, "Chat::NameServiceDataReader::return_loan");

            } else if (guardList[i].in() == leftUser ) {
               /* Some liveliness has changed (either a DataWriter joined or a DataWriter left) */
               status
                     = loadAdmin->get_liveliness_changed_status(livChangStatus);
               checkStatus(status,
                     "DDS::DataReader::get_liveliness_changed_status");

               if (livChangStatus.alive_count < prevCount) {
                  /* A user has left the ChatRoom, since a DataWriter lost its liveliness */
                  /* Take the effected users so tey will not appear in the list later on. */

                  status = nameServer->take(nsList, infoSeq,
                        LENGTH_UNLIMITED, ANY_SAMPLE_STATE, ANY_VIEW_STATE,
                        NOT_ALIVE_INSTANCE_STATE);
                  checkStatus(status, "Chat::NameServiceDataReader::take");

                  for (ULong j = 0; j < nsList.length(); j++) {
                     /* re-apply query arguments */
                     ostringstream numberString;
                     numberString << nsList[j].userID;
                     args[0UL] = numberString.str().c_str();
                     status = singleUser->set_query_parameters(args);
                     checkStatus(status,
                           "DDS::QueryCondition::set_query_parameters");

                     /* Read this users history */
                     status = loadAdmin->take_w_condition(msgList, infoSeq2,
                           LENGTH_UNLIMITED, singleUser );
                     checkStatus(status,
                           "Chat::ChatMessageDataReader::take_w_condition");

                     /* Display the user and his history */
                     cout << "Departed user " << nsList[j].name
                           << " has sent " << msgList.length()
                           << " messages." << endl;
                     status = loadAdmin->return_loan(msgList, infoSeq2);
                     checkStatus(status,
                           "Chat::ChatMessageDataReader::return_loan");
                  }
                  status = nameServer->return_loan(nsList, infoSeq);
                  checkStatus(status,
                        "Chat::NameServiceDataReader::return_loan");
               }
               prevCount = livChangStatus.alive_count;

            } else if (guardList[i].in() == escape) {
               cout << "UserLoad has terminated." << endl;
               closed = true;
            } else {
               assert(0);
            };
         } /* for */
      } /* while (!closed) */

      try {
         /* Stop the application */
         userLoadApplication.stop ();
      } catch(WrapperException& e) {
         cout << "Error while stopping the application:" << endl;
         cout << e.what() << endl;
         return -1;
      }

      CloseHandle(tHandle);

      return 0;
   }




CheckStatus.h
-------------

.. code-block:: cpp

   CheckStatus.h

   /************************************************************************
    *
    * Copyright (c) 2012 to 2016
    * PrismTech Ltd.
    * All rights Reserved.
    *
    * LOGICAL_NAME:    CheckStatus.h
    * FUNCTION:        Vortex OpenSplice Modeler Tutorial example code.
    * MODULE:          Tutorial for the C++ programming language.
    * DATE             January 2012.
    ************************************************************************
    *
    * This file contains the headers for the error handling operations.
    *
    ***/

   #ifndef __CHECKSTATUS_H__
   #define __CHECKSTATUS_H__

   #include "ccpp_dds_dcps.h"
   #include <iostream>

   /**
    * Returns the name of an error code.
    **/
   char *getErrorName(DDS::ReturnCode_t status);

   /**
    * Check the return status for errors. If there is an error, then terminate.
    **/
   void checkStatus(DDS::ReturnCode_t status, const char *info);

   /**
    * Check whether a valid handle has been returned. If not, then terminate.
    **/
   void checkHandle(void *handle, char *info);

   #endif




CheckStatus.cpp
---------------

.. code-block:: cpp

   CheckStatus.cpp

   /************************************************************************
    *
    * Copyright (c) 2012 to 2016
    * PrismTech Ltd.
    * All rights Reserved.
    *
    * LOGICAL_NAME:    CheckStatus.cpp
    * FUNCTION:        Vortex OpenSplice Modeler Tutorial example code.
    * MODULE:          Tutorial for the C++ programming language.
    * DATE             January 2012.
    ************************************************************************
    *
    * This file contains the implementation for the error handling operations.
    *
    ***/

   #include "CheckStatus.h"

   using namespace std;

   /* Array to hold the names for all ReturnCodes. */
   char *RetCodeName[13] = {
       "DDS_RETCODE_OK",
       "DDS_RETCODE_ERROR",
       "DDS_RETCODE_UNSUPPORTED",
       "DDS_RETCODE_BAD_PARAMETER",
       "DDS_RETCODE_PRECONDITION_NOT_MET",
       "DDS_RETCODE_OUT_OF_RESOURCES",
       "DDS_RETCODE_NOT_ENABLED",
       "DDS_RETCODE_IMMUTABLE_POLICY",
       "DDS_RETCODE_INCONSISTENT_POLICY",
       "DDS_RETCODE_ALREADY_DELETED",
       "DDS_RETCODE_TIMEOUT",
       "DDS_RETCODE_NO_DATA",
       "DDS_RETCODE_ILLEGAL_OPERATION" };

   /**
    * Returns the name of an error code.
    **/
   char *getErrorName(DDS::ReturnCode_t status)
   {
       return RetCodeName[status];
   }

   /**
    * Check the return status for errors. If there is an error, then terminate.
    **/
   void checkStatus(
       DDS::ReturnCode_t status,
       const char *info ) {


       if (status != DDS::RETCODE_OK && status != DDS::RETCODE_NO_DATA) {
           cerr << "Error in " << info << ": " << getErrorName(status) << endl;
           exit (0);
       }
   }

   /**
    * Check whether a valid handle has been returned. If not, then terminate.
    **/
   void checkHandle(
       void *handle,
       char *info ) {

        if (!handle) {
           cerr << "Error in " << info << ": Creation failed: invalid handle" << endl;
           exit (0);
        }
   }


.. END


.. |caution| image:: ./images/icon-caution.*
            :height: 6mm
.. |info|   image:: ./images/icon-info.*
            :height: 6mm
.. |windows| image:: ./images/icon-windows.*
            :height: 6mm
.. |unix| image:: ./images/icon-unix.*
            :height: 6mm
.. |linux| image:: ./images/icon-linux.*
            :height: 6mm
.. |c| image:: ./images/icon-c.*
            :height: 6mm
.. |cpp| image:: ./images/icon-cpp.*
            :height: 6mm
.. |csharp| image:: ./images/icon-csharp.*
            :height: 6mm
.. |java| image:: ./images/icon-java.*
            :height: 6mm

