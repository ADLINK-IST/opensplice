.. _`Appendix A`:


##########
Appendix A
##########

|java|

This appendix contains the example, user-written Java source code 
included with the Vortex OpenSplice Modeler ``Chatroom`` example. 
The Chatroom system is the example used in the
:ref:`Tutorial <Tutorial>`.

The source code is given in the following order:

  `Chatter Application`_

  `MessageBoard Application`_

  `UserLoad Application`_

  `Error Handler`_

.. _`Appx A JAVA`:

.. _`A Chatroom Example, Java Source Code`:

A Chatroom Example, Java Source Code
************************************


Chatter Application
===================

.. code-block:: java
   
   ChatterApplication.java

   /*******************************************************************************
    * Copyright (c) 2012 to 2016 PrismTech Ltd. All rights Reserved.
    * LOGICAL_NAME: ChatterApplication.java
    * FUNCTION:     Vortex OpenSplice Modeler Tutorial example code. 
    * MODULE:       Tutorial for the Java programming language. 
    * DATE:         January 2012.
    * *********************************************************************** 
    * This file contains the implementation for the 'Chatter' executable.
    ******************************************************************************/

   package Chat;

   import Chat.ChatMessage;
   import Chat.ChatMessageDataWriter;
   import Chat.NameService;
   import Chat.NameServiceDataWriter;
   import Chat.ChatterApplicationWrapper;
   import Chat.ChatterApplicationWrapper.WrapperException;
   import DDS.HANDLE_NIL;


   public class ChatterApplication
   {
      public static final int NUM_MSG = 10;

      public static final int TERMINATION_MESSAGE = -1;

      public static void main (String[] args)
      {

         try
         {
            /* Initialize the application */
            ChatterApplicationWrapper.start ();
         }
         catch (WrapperException e)
         {
            System.out.println ("Error while starting the application:");
            System.out.println (e.getReason ());
            return;
         }

         /* Type-specific DDS entities */
         ChatMessageDataWriter talker = 
               ChatterApplicationWrapper.ParticipantWrapper.PublisherWrapper.
               ChatMessageDataWriterWrapper
               .getDataWriter ();

         NameServiceDataWriter nameServer = 
               ChatterApplicationWrapper.ParticipantWrapper.PublisherWrapper.
               NameServiceDataWriterWrapper
               .getDataWriter ();

         /* DDS Identifiers */
         long userHandle;
         int status;

         /* Sample definitions */
         ChatMessage msg = new ChatMessage ();
         NameService ns = new NameService ();

         /* Others */
         int ownID = 1;
         String chatterName;

         /* Options: Chatter [ownID [name]] */
         if (args.length > 0)
         {
            ownID = Integer.parseInt (args[0]);
         }
         if (args.length > 1)
         {
            chatterName = args[1];
         }
         else
         {
            chatterName = "Chatter" + ownID;
         }

         /* Initialize the NameServer attributes */
         ns.userID = ownID;
         ns.name = chatterName;

         /*
          * Write the user-information into the system 
          *(registering the instance implicitly)
          */
         status = nameServer.write (ns, HANDLE_NIL.value);
         ErrorHandler.checkStatus (status, "Chat.NameServiceDataWriter.write");

         /* Initialize the chat messages */
         msg.userID = ownID;
         msg.index = 0;

         if (ownID == TERMINATION_MESSAGE)
         {
            msg.content = "Termination message.";
         }
         else
         {
            msg.content = "Hi there, I will send you " + NUM_MSG + " more messages.";
         }
         System.out.println ("Writing message: \"" + msg.content + "\"");

         /*
          * Register a chat message for this user (pre-allocating resources for
          * it!!)
          */
         userHandle = talker.register_instance (msg);

         /* Write a message using the pre-generated instance handle */
         status = talker.write (msg, userHandle);
         ErrorHandler.checkStatus (status, "Chat.ChatMessageDataWriter.write");

         /* Write any number of messages */
         for (int i = 1; i <= NUM_MSG && ownID != TERMINATION_MESSAGE; i++)
         {
            try
            {
               Thread.sleep (1000); /* do not run so fast! */
            }
            catch (InterruptedException e)
            {
               e.printStackTrace ();
            }
            msg.index = i;
            msg.content = "Message no. " + i;
            System.out.println ("Writing message: \"" + msg.content + "\"");
            status = talker.write (msg, userHandle);
            ErrorHandler.checkStatus (status, "Chat.ChatMessageDataWriter.write");
         }

         /* Unregister the message instance for this user explicitly */
         status = talker.dispose(msg, userHandle);
         ErrorHandler.checkStatus(status, "Chat.ChatMessageDataWriter.dispose");
         status = talker.unregister_instance (msg, userHandle);
         ErrorHandler.checkStatus (status, 
            "Chat.ChatMessageDataWriter.unregister_instance");

         /* Leave the room */
         status = nameServer.unregister_instance(ns, HANDLE_NIL.value);
         ErrorHandler.checkStatus (status, "Chat.NameServiceDataWriter.dispose");

         try
         {
            /* Stop the application */
            ChatterApplicationWrapper.stop ();
         }
         catch (WrapperException e)
         {
            System.out.println ("Error while stopping the application:");
            System.out.println (e.getReason ());
            return;
         }
      }
   }


MessageBoard Application
========================

.. code-block:: java
   

   MessageBoardApplication.java

   /************************************************************************
    *
    * Copyright (c) 2012 to 2016
    * PrismTech Ltd.
    * All rights Reserved.
    *
    * LOGICAL_NAME:    MessageBoardApplication.java
    * FUNCTION:        Vortex OpenSplice Modeler Tutorial example code.
    * MODULE:          Tutorial for the Java programming language.
    * DATE             January 2012.
    ************************************************************************
    *
    * This file contains the implementation for the 'MessageBoardApplication' 
    * executable.
    *
    ***/

   package Chat;

   import Chat.NamedMessageFilteredTopicWrapper;
   import Chat.MessageBoardApplicationWrapper;
   import Chat.MessageBoardApplicationWrapper.WrapperException;


   public class MessageBoardApplication extends Thread
   {
      public static void main (String[] args)
      {
         /* DDS Identifiers */
         String ownID = "0";

         /* Options: MessageBoard [ownID] */
         /* Messages having owner ownID will be ignored */
         if (args.length > 0)
         {
            ownID = args[0];
         }

         /* Initialize the content filtered topics expression parameters */
         try
         {
            NamedMessageFilteredTopicWrapper.setExpressionParameters (new String[]
            {
               ownID
            });
         }
         catch (NamedMessageFilteredTopicWrapper.WrapperException e)
         {
            System.out.println (
               "Exception occurred while setting the expression parameters:");
            System.out.println (e.getReason ());
            return;
         }

         /* Initialize the application */
         try
         {
            MessageBoardApplicationWrapper.start ();
         }
         catch (WrapperException e)
         {
            System.out.println ("Exception occurred while starting 
               the application:");
            System.out.println (e.getReason ());
            return;
         }

         /* Create the listeners for the MessageBoard application */
         ChatMessageDataReaderListenerImpl chatMessageDataReaderListener = 
             new ChatMessageDataReaderListenerImpl ();

         NamedMessageDataReaderListenerImpl namedMessageDataReaderListener = 
             new NamedMessageDataReaderListenerImpl ();

         /* Print a message that the MessageBoard has opened. */
         System.out.println (
          "MessageBoard has opened: send ChatMessage with userID = -1 to close it.");
         System.out.println ();

         try
         {
            /* Attach the ChatMessageDataReaderListener to the 
               ChatMessageDataReader */
            MessageBoardApplicationWrapper.PrivateParticipantWrapper
               .SubscriberWrapper.ChatMessageDataReaderWrapper.attach (
               chatMessageDataReaderListener);

            /*
             * Attach the NamedMessageDataReaderListener to the
             * NamedMessageDataReader
             */
            MessageBoardApplicationWrapper.ParticipantWrapper.SubscriberWrapper.
               NamedMessageDataReaderWrapper.attach (namedMessageDataReaderListener);

         }
         catch (WrapperException e)
         {
            System.out.println ("Exception occured while attaching a listener:");
            System.out.println (e.getReason ());
            try
            {
               MessageBoardApplicationWrapper.stop ();
            }
            catch (WrapperException eStop)
            {
               System.out.println ("Exception occured while stopping application:");
               System.out.println (eStop.getReason ());
            }
            return;
         }


         /* Wait for the ChatMessageDataReaderListener to finish */
         while (!chatMessageDataReaderListener.isTerminated ())
         {
            /*
             * Sleep for some amount of time, as not to consume too much CPU
             * cycles.
             */
            try
            {
               Thread.sleep (1000);
            }
            catch (InterruptedException e)
            {
               e.printStackTrace ();
            }
         }

         /* Wait for the NamedMessageDataReaderListener to finish */
         while (!namedMessageDataReaderListener.isTerminated ())
         {
            /*
             * Sleep for some amount of time, as not to consume too much CPU
             * cycles.
             */
            try
            {
               Thread.sleep (1000);
            }
            catch (InterruptedException e)
            {
               e.printStackTrace ();
            }
         }

         /* Print a message that the MessageBoard has terminated */
         System.out.println ("Termination message received: exiting...");
         
         try
         {
            /* Detach the ChatMessageDataReaderListener to ChatMessageDataReader */
            MessageBoardApplicationWrapper.PrivateParticipantWrapper
               .SubscriberWrapper.ChatMessageDataReaderWrapper.detach (
               chatMessageDataReaderListener);
            
            /*
             * Detach the NamedMessageDataReaderListener to the
             * NamedMessageDataReader
             */
            MessageBoardApplicationWrapper.ParticipantWrapper.SubscriberWrapper.
               NamedMessageDataReaderWrapper.detach (namedMessageDataReaderListener);
         }
         catch (WrapperException e)
         {
            System.out.println ("Exception occurred while detaching the listeners:");
            System.out.println (e.getReason ());
         }

         /* Cleanup listener */
         chatMessageDataReaderListener.cleanup ();

         /* Stop the application */
         try
         {
            MessageBoardApplicationWrapper.stop ();
         }
         catch (WrapperException e)
         {
            System.out.println ("Exception occurred while stopping application:");
            System.out.println (e.getReason ());
         }
      }
   }

ChatMessageDataReaderListenerImpl.java
--------------------------------------

.. code-block:: java
   
   ChatMessageDataReaderListenerImpl.java

   /************************************************************************
    * Copyright (c) 2012 to 2016 PrismTech Ltd. All rights Reserved.
    * LOGICAL_NAME: ChatMessageDataReaderListenerImpl.java
    * FUNCTION:     Vortex OpenSplice Modeler Tutorial example code
    * MODULE:       Tutorial for the Java programming language
    * DATE:         January 2012
    * This file contains the implementation for the 'MessageBoard' executable
    ***/

   package Chat;

   import Chat.ChatMessageDataReader;
   import Chat.ChatMessageSeqHolder;
   import Chat.NameServiceDataReader;
   import Chat.NameServiceSeqHolder;
   import Chat.NamedMessage;
   import Chat.NamedMessageDataWriter;
   import DDS.ANY_INSTANCE_STATE;
   import DDS.ANY_SAMPLE_STATE;
   import DDS.ANY_VIEW_STATE;
   import DDS.DataReader;
   import DDS.HANDLE_NIL;
   import DDS.LENGTH_UNLIMITED;
   import DDS.QueryCondition;
   import DDS.RETCODE_NO_DATA;
   import DDS.ReadCondition;
   import DDS.SampleInfoSeqHolder;
   import Chat.MessageBoardApplicationWrapper;
   import Chat.MessageBoardApplicationWrapper.ChatMessageDataReaderListener;


   public class ChatMessageDataReaderListenerImpl extends 
   ChatMessageDataReaderListener
   {
      private static final int TERMINATION_MESSAGE = -1;

      private boolean isTerminated;

      /* Generic DDS entities */
      private QueryCondition nameFinder;

      private ReadCondition newMessages;

      /* Type-specific DDS entities */
      private ChatMessageDataReader chatMsgReader;

      private NameServiceDataReader nameServiceReader;

      private NamedMessageDataWriter namedMessageWriter;

      private ChatMessageSeqHolder chatMsgSeq;

      private SampleInfoSeqHolder chatMsgInfoSeq;

      private NameServiceSeqHolder nameServiceSeq;

      private SampleInfoSeqHolder nameServiceInfoSeq;

      /* Others */
      private String nameFinderExpr;

      private String[] nameFinderParams;

      private String userName;

      private int previousID;

      /* DDS Identifiers */
      private int status;

      /* Sample definitions */
      NamedMessage namedMsg;

      public ChatMessageDataReaderListenerImpl ()
      {
         /* Initialize termination flag */
         setTerminated (false);

         /* Type-specific DDS entities */
         chatMsgReader = MessageBoardApplicationWrapper.PrivateParticipantWrapper
            .SubscriberWrapper.ChatMessageDataReaderWrapper.getDataReader ();
         nameServiceReader = MessageBoardApplicationWrapper.PrivateParticipantWrapper
            .SubscriberWrapper.NameServiceDataReaderWrapper.getDataReader ();
         namedMessageWriter = MessageBoardApplicationWrapper
            .PrivateParticipantWrapper.PublisherWrapper.NamedMessageDataWriterWrapper
            .getDataWriter ();
         chatMsgSeq = new ChatMessageSeqHolder ();
         chatMsgInfoSeq = new SampleInfoSeqHolder ();
         nameServiceSeq = new NameServiceSeqHolder ();
         nameServiceInfoSeq = new SampleInfoSeqHolder ();

         /* Others */
         nameFinderExpr = "userID = %0";
         nameFinderParams = new String[]
         {
            "0"
         };
         userName = "";
         previousID = -1;

         /* Sample definitions */
         namedMsg = new NamedMessage ();

         /*
          * Create a QueryCondition that will look up the userName for a
          * specified userID
          */
         nameFinder = nameServiceReader.create_querycondition (
            ANY_SAMPLE_STATE.value, ANY_VIEW_STATE.value, ANY_INSTANCE_STATE.value,
            nameFinderExpr, nameFinderParams);
         ErrorHandler.checkHandle (nameFinder, 
            "Chat.NameServiceDataReader.create_querycondition");

         newMessages = chatMsgReader.create_readcondition (ANY_SAMPLE_STATE.value, 
            ANY_VIEW_STATE.value, ANY_INSTANCE_STATE.value);
         ErrorHandler.checkHandle (newMessages, 
            "Chat.ChatMessageDataReader.create_readcondition");
      }

      @Override
      public void on_data_available(DataReader dataReader) {
         /* Ignore new data if termination message already received */
         if (isTerminated) {
            return;
         }

         boolean terminationReceived = false;

         if (dataReader.equals(chatMsgReader)) {
            status = chatMsgReader.take_w_condition(chatMsgSeq, chatMsgInfoSeq,
                  LENGTH_UNLIMITED.value, newMessages);
            ErrorHandler.checkStatus(status,
                  "Chat.ChatMessageDataReader.take_w_condition");

            /*
             * For each message, extract the key-field and find the
             * corresponding name
             */
            for (int i = 0; i < chatMsgSeq.value.length; i++) {
               /*
                * Set program termination flag if termination message is
                * received
                */
               if (chatMsgSeq.value[i].userID == TERMINATION_MESSAGE) {
                  terminationReceived = true;
                  break;
               }

               /* Find the corresponding named message */
               if (chatMsgSeq.value[i].userID != previousID) {
                  previousID = chatMsgSeq.value[i].userID;
                  nameFinderParams[0] = Integer.toString(previousID);
                  status = nameFinder.set_query_parameters(nameFinderParams);
                  ErrorHandler
                        .checkStatus(status,
                           "QueryCondition.set_query_arguments (nameFinderParams)");
                  status = nameServiceReader.read_w_condition(nameServiceSeq,
                        nameServiceInfoSeq, LENGTH_UNLIMITED.value,
                        nameFinder);
                  ErrorHandler.checkStatus(status,
                        "Chat.NameServiceDataReader.read_w_condition");

                  /* Extract Name (there should only be one result) */
                  if (status == RETCODE_NO_DATA.value) {
                     userName = "Name not found!! id = " + previousID;
                  } else {
                     userName = nameServiceSeq.value[0].name;
                  }

                  /* Release the name sample again */
                  status = nameServiceReader.return_loan(nameServiceSeq,
                        nameServiceInfoSeq);
                  ErrorHandler.checkStatus(status,
                        "Chat.NameServiceDataReader.return_loan");
               }
               /* Write merged Topic with userName instead of userID */
               namedMsg.userName = userName;
               namedMsg.userID = previousID;
               namedMsg.index = chatMsgSeq.value[i].index;
               namedMsg.content = chatMsgSeq.value[i].content;
               
               if (chatMsgInfoSeq.value[i].valid_data)
               {
                  status = namedMessageWriter.write (namedMsg, HANDLE_NIL.value);
                  ErrorHandler.checkStatus (status, 
                     "Chat.NamedMessageDataWriter.write");
               }
            }

            status = chatMsgReader.return_loan(chatMsgSeq, chatMsgInfoSeq);
            ErrorHandler.checkStatus(status,
                  "Chat.ChatMessageDataReader.return_loan");

            if (terminationReceived) {
               setTerminated(true);
            }
         }
      }

      public void cleanup ()
      {
         /* Remove all Read Conditions from the DataReaders */
         status = nameServiceReader.delete_readcondition (nameFinder);
         ErrorHandler.checkStatus (status, "Chat.NameServiceDataReader
         .delete_readcondition(nameFinder)");
         status = chatMsgReader.delete_readcondition (newMessages);
         ErrorHandler.checkStatus (status, "Chat.ChatMessageDataReader
         .delete_readcondition(newMessages)");
      }

      public synchronized boolean isTerminated ()
      {
         return isTerminated;
      }

      private synchronized void setTerminated (boolean isTerminated)
      {
         this.isTerminated = isTerminated;
      }
   }

NamedMessageDataReaderListenerImpl.java
---------------------------------------

.. code-block:: java
   
   NamedMessageDataReaderListenerImpl.java

   /************************************************************************
    *
    * Copyright (c) 2012 to 2016
    * PrismTech Ltd.
    * All rights Reserved.
    *
    * LOGICAL_NAME:    NamedMessageDataReaderListenerImpl.java
    * FUNCTION:        Vortex OpenSplice Modeler Tutorial example code.
    * MODULE:          Tutorial for the Java programming language.
    * DATE             January 2012.
    ************************************************************************
    *
    * This file contains the implementation for the 'MessageBoard' executable.
    *
    ***/

   package Chat;

   import Chat.NamedMessageDataReader;
   import Chat.NamedMessageSeqHolder;
   import DDS.ALIVE_INSTANCE_STATE;
   import DDS.ANY_VIEW_STATE;
   import DDS.DataReader;
   import DDS.LENGTH_UNLIMITED;
   import DDS.NOT_READ_SAMPLE_STATE;
   import DDS.SampleInfoSeqHolder;
   import Chat.MessageBoardApplicationWrapper;
   import Chat.MessageBoardApplicationWrapper.NamedMessageDataReaderListener;

   public class NamedMessageDataReaderListenerImpl extends
         NamedMessageDataReaderListener {
      private boolean isTerminated;

      /* DDS Identifiers */
      private int status;

      /* Type-specific DDS entities */
      private NamedMessageDataReader namedMsgReader;
      private NamedMessageSeqHolder namedMsgSeq;
      private SampleInfoSeqHolder infoSeq;

      public NamedMessageDataReaderListenerImpl() {
         namedMsgReader = MessageBoardApplicationWrapper.ParticipantWrapper
            .SubscriberWrapper.NamedMessageDataReaderWrapper
            .getDataReader();
         namedMsgSeq = new NamedMessageSeqHolder();
         infoSeq = new SampleInfoSeqHolder();
      }

      @Override
      public void on_data_available(DataReader dataReader) {
         /* Set termination flag */
         setTerminated(false);

         status = namedMsgReader.take(namedMsgSeq, infoSeq,
               LENGTH_UNLIMITED.value, NOT_READ_SAMPLE_STATE.value,
               ANY_VIEW_STATE.value, ALIVE_INSTANCE_STATE.value);
         ErrorHandler.checkStatus(status, "Chat.NamedMessageDataReader.read");

         /* For each message, print the message */
         for (int i = 0; i < namedMsgSeq.value.length; i++) {
            System.out.println(namedMsgSeq.value[i].userName + ": "
                  + namedMsgSeq.value[i].content);
         }

         status = namedMsgReader.return_loan(namedMsgSeq, infoSeq);
         ErrorHandler.checkStatus(status,
               "Chat.NamedMessageDataReader.return_loan");

         namedMsgSeq.value = null;
         infoSeq.value = null;

         /* Unset termination flag */
         setTerminated(true);
      }

      public synchronized boolean isTerminated() {
         return isTerminated;
      }

      private synchronized void setTerminated(boolean isTerminated) {
         this.isTerminated = isTerminated;
      }
   }


UserLoad Application
====================

.. code-block:: java
   
   
   UserLoadApplication.java

   /*******************************************************************************

    * Copyright (c) 2012 to 2016
    * PrismTech Ltd.
    * All rights Reserved.
    *
    * LOGICAL_NAME: UserLoadApplication.java
    * FUNCTION:     Vortex OpenSplice Modeler Tutorial example code.
    * MODULE:       Tutorial for the Java programming language.
    * DATE          January 2012.
    * ***********************************************************************
    *
    * This file contains the implementation for the 'UserLoadApplication' executable.
    *
    ******************************************************************************/

   package Chat;

   import Chat.UserLoadApplicationWrapper.WrapperException;
   import DDS.*;


   public class UserLoadApplication extends Thread
   {
      /* entities required by all threads */
      private static GuardCondition escape;

      /**
       * Sleeper thread: sleeps 60 seconds and then triggers the WaitSet
       */
      public void run ()
      {
         int status;

         try
         {
            sleep (60000);
         }
         catch (InterruptedException e)
         {
            e.printStackTrace ();
         }
         status = escape.set_trigger_value (true);
         ErrorHandler.checkStatus (status, "DDS.GuardCondition.set_trigger_value");
      }

      public static void main (String[] args)
      {

         boolean closed = false;
         int prevCount = 0;

         try
         {
            /* Initialize the application */
            UserLoadApplicationWrapper.start ();
         }
         catch (WrapperException e)
         {
            System.out.println ("Error while starting the application:");
            System.out.println (e.getReason ());
            return;
         }

         /* Initialize the arguments and params for the QueryCondition */
         String[] params;

         params = new String[]
         {
            "0"
         };

         try
         {
            UserLoadApplicationWrapper.QueryConditionWrapper.setQueryParameters (
               params);

            /* start the ChatMessageDataReaderWaitSet */
            UserLoadApplicationWrapper.UserLoadWaitSetWrapper.start ();
         }
         catch (WrapperException e)
         {
            System.out.println ("Error while initializing the application:");
            System.out.println (e.getReason ());
            return;
         }

         WaitSet userLoadWS = UserLoadApplicationWrapper.UserLoadWaitSetWrapper
            .getWaitSet ();

         /* Generic DDS entities */
         escape = UserLoadApplicationWrapper.GuardConditionWrapper
            .getGuardCondition ();
         QueryCondition singleUser = UserLoadApplicationWrapper
            .QueryConditionWrapper.getQueryCondition ();
         ReadCondition newUser = UserLoadApplicationWrapper.ReadConditionWrapper
            .getReadCondition ();
         StatusCondition leftUser = UserLoadApplicationWrapper.StatusConditionWrapper
            .getStatusCondition ();

         LivelinessChangedStatusHolder livChangedStatusHolder = new 
            LivelinessChangedStatusHolder ();

         /* DDS Identifiers */
         int status;
         ConditionSeqHolder guardList = new ConditionSeqHolder ();

         /* Type-specific DDS entities */
         NameServiceDataReader nameServer = UserLoadApplicationWrapper
            .ParticipantWrapper.SubscriberWrapper.NameServiceDataReaderWrapper
            .getDataReader ();
         ChatMessageDataReader loadAdmin = UserLoadApplicationWrapper
            .ParticipantWrapper.SubscriberWrapper.ChatMessageDataReaderWrapper
            .getDataReader ();
         ChatMessageSeqHolder msgList = new ChatMessageSeqHolder ();
         NameServiceSeqHolder nsList = new NameServiceSeqHolder ();
         SampleInfoSeqHolder infoSeq = new SampleInfoSeqHolder ();
         SampleInfoSeqHolder infoSeq2 = new SampleInfoSeqHolder ();

         /*
          * Initialize and pre-allocate the GuardList used to obtain the triggered
          * Conditions.
          */
         guardList.value = new Condition[3];

         /* Remove all known Users that are not currently active. */
         status = nameServer.take (nsList, infoSeq, LENGTH_UNLIMITED.value, 
            ANY_SAMPLE_STATE.value, ANY_VIEW_STATE.value,
            NOT_ALIVE_INSTANCE_STATE.value);
         ErrorHandler.checkStatus (status, "Chat.NameServiceDataReader.take");
         status = nameServer.return_loan (nsList, infoSeq);
         ErrorHandler.checkStatus (status, "Chat.NameServiceDataReader.return_loan");

         /* Start the sleeper thread */
         new UserLoadApplication ().start ();

         while (!closed)
         {
            /* Wait until at least one of the Conditions in the waitset triggers */
            status = userLoadWS._wait (guardList, DURATION_INFINITE.value);
            ErrorHandler.checkStatus (status, "DDS.WaitSet._wait");

            /* Walk over all the guards to display information */
            for (int i = 0; i < guardList.value.length; i++)
            {
               if (guardList.value[i] == newUser)
               {
                  /* The newUser ReadCondition contains data */
                  status = nameServer.read_w_condition (nsList, infoSeq, 
                     LENGTH_UNLIMITED.value, newUser);
                  ErrorHandler.checkStatus (status, 
                     "Chat.NameServiceDataReader.read_w_condition");

                  for (int j = 0; j < nsList.value.length; j++)
                  {
                     System.out.println ("New User: " + nsList.value[j].name);
                  }
                  status = nameServer.return_loan (nsList, infoSeq);
                  ErrorHandler.checkStatus (status, 
                     "Chat.NameServiceDataReader.return_loan");
               }
               else if (guardList.value[i] == leftUser)
               {
                  /*
                   * Some liveliness has changed (either because a DataWriter
                   * joined or a DataWriter left
                   */
                  status = loadAdmin.get_liveliness_changed_status (
                     livChangedStatusHolder);
                  ErrorHandler.checkStatus (status, 
                     "Chat.ChatMessageDataReader.get_liveliness_changed_status");

                  if (livChangedStatusHolder.value.alive_count < prevCount)
                  {
                     /*
                      * A user has left the ChatRoom, since a DataWriter lost its
                      * liveliness
                      */
                     /*
                      * Take the effected users so they will not appear in the list
                      * later on
                      */
                     status = nameServer.take (nsList, infoSeq, 
                        LENGTH_UNLIMITED.value, ANY_SAMPLE_STATE.value, 
                        ANY_VIEW_STATE.value, NOT_ALIVE_INSTANCE_STATE.value);
                     ErrorHandler.checkStatus (status, 
                        "Chat.NameServiceDataReader.take");

                     for (int j = 0; j < nsList.value.length; j++)
                     {
                        /* re-apply query arguments */
                        params[0] = Integer.toString (nsList.value[j].userID);
                        status = singleUser.set_query_parameters (params);
                        ErrorHandler.checkStatus (status, 
                           "DDS.QueryCondition.set_query_arguments");

                        /* Read this users history */
                        status = loadAdmin.take_w_condition (msgList, infoSeq2, 
                           LENGTH_UNLIMITED.value, singleUser);
                        ErrorHandler.checkStatus (status, 
                           "Chat.ChatMessageDataReader.read_w_condition");

                        /* Display the user and his history */
                        System.out.println ("Departed user " + nsList.value[j].name +
                            " had sent " + msgList.value.length + " messages.");
                        status = loadAdmin.return_loan (msgList, infoSeq2);
                        ErrorHandler.checkStatus (status, 
                           "Chat.ChatMessageDataReader.return_loan");
                        msgList.value = null;
                        infoSeq2.value = null;
                     }
                     status = nameServer.return_loan (nsList, infoSeq);
                     ErrorHandler.checkStatus (status, 
                        "Chat.NameServiceDataReader.return_loan");
                     nsList.value = null;
                     infoSeq.value = null;
                  }
                  prevCount = livChangedStatusHolder.value.alive_count;
               }
               else if (guardList.value[i] == escape)
               {
                  System.out.println ("UserLoad has terminated.");
                  closed = true;
               }
               else
               {
                  assert false : "Unknown Condition";
               }
            } /* for */
         } /* while (!closed) */

         try
         {
            /* Stop the application and free all resources */
            UserLoadApplicationWrapper.stop ();
         }
         catch (WrapperException e)
         {
            System.out.println ("Error while stopping the application:");
            System.out.println (e.getReason ());
            return;
         }
      }
   }


Error Handler
=============

.. code-block:: java
   
   ErrorHandler.java

   /************************************************************************
    * 
    * Copyright (c) 2012 to 2016
    * PrismTech Ltd. 
    * All rights Reserved.
    * 
    * LOGICAL_NAME: ErrorHandler.java 
    * FUNCTION: Vortex OpenSplice Modeler Tutorial example code. 
    * MODULE: Tutorial for the Java programming language. 
    * DATE: January 2012.
    ************************************************************************ 
    * 
    * This file contains the implementation for the error handling operations.
    * 
    ***/

   package Chatroom;

   import DDS.RETCODE_NO_DATA;
   import DDS.RETCODE_OK;


   public class ErrorHandler
   {
      public static final int NR_ERROR_CODES = 12;

      /* Array to hold the names for all ReturnCodes. */
      public static String[] RetCodeName = new String[NR_ERROR_CODES];

      static
      {
         RetCodeName[0] = new String ("DDS_RETCODE_OK");
         RetCodeName[1] = new String ("DDS_RETCODE_ERROR");
         RetCodeName[2] = new String ("DDS_RETCODE_UNSUPPORTED");
         RetCodeName[3] = new String ("DDS_RETCODE_BAD_PARAMETER");
         RetCodeName[4] = new String ("DDS_RETCODE_PRECONDITION_NOT_MET");
         RetCodeName[5] = new String ("DDS_RETCODE_OUT_OF_RESOURCES");
         RetCodeName[6] = new String ("DDS_RETCODE_NOT_ENABLED");
         RetCodeName[7] = new String ("DDS_RETCODE_IMMUTABLE_POLICY");
         RetCodeName[8] = new String ("DDS_RETCODE_INCONSISTENT_POLICY");
         RetCodeName[9] = new String ("DDS_RETCODE_ALREADY_DELETED");
         RetCodeName[10] = new String ("DDS_RETCODE_TIMEOUT");
         RetCodeName[11] = new String ("DDS_RETCODE_NO_DATA");
      }

      /**
       * Returns the name of an error code.
       */
      public static String getErrorName (int status)
      {
         return RetCodeName[status];
      }

      /**
       * Check the return status for errors. If there is an error, then terminate.
       */
      public static void checkStatus (int status, String info)
      {
         if (status != RETCODE_OK.value && status != RETCODE_NO_DATA.value)
         {
            System.out.println ("Error in " + info + ": " + getErrorName (status));
            System.exit (-1);
         }
      }

      /**
       * Check whether a valid handle has been returned. If not, then terminate.
       */
      public static void checkHandle (Object handle, String info)
      {
         if (handle == null)
         {
            System.out.println ("Error in " + info + 
               ": Creation failed: invalid handle");
            System.exit (-1);
         }
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
