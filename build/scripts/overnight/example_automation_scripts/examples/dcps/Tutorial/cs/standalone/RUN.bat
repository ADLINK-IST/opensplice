@echo off

call %FUNCTIONS% :stopOSPL

call %FUNCTIONS% :startOSPL

ECHO Starting MessageBoard
SET LEVEL=Starting MessageBoard
start CMD /C sacs_tutorial_message_board.exe ^> messageBoard.log
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting UserLoad
SET LEVEL=Starting UserLoad
start CMD /C sacs_tutorial_user_load.exe ^> userLoad.log
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting UserLoad %ERRORLEVEL%

%SLEEP4% >NUL

ECHO Starting Chatter
SET LEVEL=Staring Chatter
sacs_tutorial_chatter.exe >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting Chatter with terminate message
SET LEVEL=Starting Chatter with -1
sacs_tutorial_chatter.exe -1 >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 GOTO error
ECHO Waiting for UserLoad to terminate naturally
GOTO while1

:while1
REM Check to see if the user load exe is still running and wait for it to stop
REM the tasklist command returns an abridged imagename and so we need to check
REM for that and not the full name - this is not working with the overnight runs
REM so lets display what's is returned by tasklist to see why it's not working
tasklist | sort 
tasklist /FI "IMAGENAME eq sacs_tutorial_user_load.exe" 2>NUL | findstr "sacs_tutorial_user_load.e">NUL
if "%ERRORLEVEL%"=="0" (
   ECHO .
   %SLEEP5%>NUL
   GOTO while1
) else (
   ECHO UserLoad not running
   GOTO end
)

:error
ECHO An error occurred %LEVEL%, exiting example ... >> %LOGFILE%
REM Do not exit as this prevents the test run reporting correctly
GOTO end

:end
call %FUNCTIONS% :stopOSPL

