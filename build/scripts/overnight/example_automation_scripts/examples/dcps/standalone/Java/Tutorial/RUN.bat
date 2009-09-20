@ECHO OFF
set SLEEP2=@ping -n 2 localhost
set SLEEP4=@ping -n 4 localhost
set SLEEP10=@ping -n 10 localhost

REM start OpenSplice

ospl start 

%SLEEP10% >NUL

REM start MessageBoard
start java -classpath "%OSPL_HOME%\jar\dcpssaj.jar;bld" chatroom.MessageBoard 

%SLEEP4% >NUL

ECHO start Chatter
call java -classpath "%OSPL_HOME%\jar\dcpssaj.jar;bld" chatroom.Chatter

%SLEEP2% >NUL

ECHO start Chatter with terminate message
call java -classpath "%OSPL_HOME%\jar\dcpssaj.jar;bld" chatroom.Chatter -1

%SLEEP4% >NUL

ospl stop
