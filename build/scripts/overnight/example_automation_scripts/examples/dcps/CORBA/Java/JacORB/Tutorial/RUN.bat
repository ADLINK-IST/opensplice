@ECHO OFF

set SLEEP4=@C:\WINDOWS\system32\ping.exe -n 4 localhost

REM start OpenSplice

ECHO Starting ospl
SET LEVEL=Staring ospl
ospl start 
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting ospl %ERRORLEVEL%

%SLEEP4% >NUL

ECHO Starting MessageBoard
SET LEVEL=Starting MessageBoard
start java -classpath "%OSPL_HOME%\jar\dcpscj.jar;bld" chatroom.MessageBoard 
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting MessageBoard %ERRORLEVEL%

%SLEEP4% >NUL

ECHO Starting UserLoad
SET LEVEL=Starting UserLoad
start java -classpath "%OSPL_HOME%\jar\dcpssaj.jar;bld" chatroom.UserLoad
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting UserLoad %ERRORLEVEL%

%SLEEP4% >NUL

ECHO Starting Chatter
SET LEVEL=Starting Chatter
call java -classpath "%OSPL_HOME%\jar\dcpscj.jar;bld" chatroom.Chatter
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting Chatter with terminate message
SET LEVEL=Starting Chatter with -1
call java -classpath "%OSPL_HOME%\jar\dcpscj.jar;bld" chatroom.Chatter -1
if %ERRORLEVEL% NEQ 0 GOTO error

GOTO end

:error
ECHO An error occurred %LEVEL%, exiting example .....
REM Do not exit as this prevents the test run reporting correctly
GOTO end

:end

ospl stop
if %ERRORLEVEL% NEQ 0 ECHO Error stopping ospl

%SLEEP4% >NUL