@ECHO OFF

set SLEEP4=@C:\WINDOWS\system32\ping.exe -n 4 localhost

REM start OpenSplice

IF NOT EXIST ".\MessageBoard\MessageBoard.exe" (
   ECHO Error: MessageBoard.exe not exist
   GOTO error
)

IF NOT EXIST "Chatter\Chatter.exe" (
   ECHO Error: Chatter.exe not exist
   GOTO error
)

ECHO Starting ospl
SET LEVEL=Starting ospl
ospl start 
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting ospl %ERRORLEVEL%

%SLEEP4% >NUL

ECHO Starting MessageBoard
SET LEVEL=Starting MessageBoard
start "Messageboard" .\MessageBoard\MessageBoard.exe
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting MessageBoard %ERRORLEVEL%

%SLEEP4% >NUL

ECHO Starting UserLoad
SET LEVEL=Starting UserLoad
start "UserLoad" .\UserLoad\UserLoad.exe
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting UserLoad %ERRORLEVEL%

%SLEEP4% >NUL

ECHO Starting Chatter
SET LEVEL=Staring Chatter
call .\Chatter\Chatter.exe
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting Chatter with terminate message
SET LEVEL=Starting Chatter with -1
call .\Chatter\Chatter.exe -1
if %ERRORLEVEL% NEQ 0 GOTO error

GOTO end

:error
ECHO An error occurred %LEVEL%, exiting example ...
REM Do not exit as this prevents the test run reporting correctly
GOTO end

:end

ospl stop
if %ERRORLEVEL% NEQ 0 ECHO Error stopping ospl

%SLEEP4% >NUL