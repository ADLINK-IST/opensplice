The tutorial consists of three executables that together make up a primitive
chatbox:
* Chatter: Use this executable to send chat messages to the chatbox.
* MessageBoard: Use this executable to view all available chatmessages.
* UserLoad: Use this executable to keep track of when users join and leave
  the chatbox.

Some executables support optional command-line parameters. When these
parameters are not specified, dedault values are used instead. The
optional parameters and their meaning are specified below:

Chatter [userid] [username]
  userid:   an integer number that uniquely identifies the sender of a message
            (Transmit a message with userid = -1 to terminate the MessageBoard.)
  username: the user-name other chatters will see when they receive one of your
            chat messages.

MessageBoard [userid]
  userid:   block messages from a user identified by this id. You can use this
            to prevent seeing your own messages appear on your own
            MessageBoard.

The executables classes are located in the chatroom package, but should be
started from the current directory in the following way:

java -classpath $OSPL_HOME/jar/dcpscj.jar:bld chatroom.MessageBoard

optionally command-line parameters may be included as well:

java -classpath $OSPL_HOME/jar/dcpscj.jar:bld chatroom.Chatter 1 Bill
