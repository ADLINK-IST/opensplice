@ECHO OFF

set SLEEP4=@C:\WINDOWS\system32\ping.exe -n 4 localhost

set PATH=%~dp0\MessageBoard\Release;%~dp0\MessageBoard\Debug;%PATH%
set PATH=%~dp0\Chatter\Release;%~dp0\Chatter\Debug;%PATH%
set PATH=%~dp0\UserLoad\Release;%~dp0\UserLoad\Debug;%PATH%

REM start OpenSplice

ECHO Starting ospl
SET LEVEL=Starting ospl
ospl start 
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting ospl %ERRORLEVEL%

%SLEEP4% >NUL

ECHO Starting MessageBoard
SET LEVEL=Starting MessageBoard
start "Messageboard" MessageBoard.exe
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting MessageBoard %ERRORLEVEL%

%SLEEP4% >NUL

ECHO Starting UserLoad
SET LEVEL=Starting UserLoad
start "UserLoad" UserLoad.exe
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting UserLoad %ERRORLEVEL%

%SLEEP4% >NUL

ECHO Starting Chatter
SET LEVEL=Starting Chatter
call Chatter.exe
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting Chatter with terminate message
SET LEVEL=Starting Chatter with -1
call Chatter.exe -1
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

GOTO end

:error
ECHO An error occurred %LEVEL%, exiting example ....
REM Do not exit as this prevents the test run reporting correctly
GOTO end

:end

ospl stop
if %ERRORLEVEL% NEQ 0 ECHO Error stopping ospl %ERRORLEVEL%

%SLEEP4% >NUL