@ECHO OFF

REM start OpenSplice

IF NOT EXIST ".\MessageBoard\Release\MessageBoard.exe" (
   ECHO Error: MessageBoard.exe not exist
   GOTO error
)

IF NOT EXIST "Chatter\Release\Chatter.exe" (
   ECHO Error: Chatter.exe not exist
   GOTO error
)

ECHO Starting ospl
SET LEVEL=Starting ospl
ospl start 
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting ospl %ERRORLEVEL%

sleep 4

ECHO Starting MessageBoard
SET LEVEL=Starting MessageBoard
start "Messageboard" .\MessageBoard\Release\MessageBoard.exe
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting MessageBoard %ERRORLEVEL%

sleep 4

ECHO Starting Chatter
SET LEVEL=Starting Chatter
call .\Chatter\Release\Chatter.exe
if %ERRORLEVEL% NEQ 0 GOTO error

sleep 4

ECHO Starting Chatter with terminate message
SET LEVEL=Starting Chatter with -1
call .\Chatter\Release\Chatter.exe -1
if %ERRORLEVEL% NEQ 0 GOTO error


GOTO end

:error
ECHO An error occurred %LEVEL%, exiting example ...
REM Do not exit as this prevents the test run reporting correctly
GOTO end

:end

ospl stop
if %ERRORLEVEL% NEQ 0 ECHO Error stopping ospl

sleep 4