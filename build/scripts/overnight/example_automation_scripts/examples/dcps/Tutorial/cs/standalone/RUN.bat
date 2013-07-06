@echo off

call %FUNCTIONS% :stopOSPL

call %FUNCTIONS% :deleteDBFFiles

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
%SLEEP10% > NUL
GOTO end

:error
ECHO An error occurred %LEVEL%, exiting example ... >> %LOGFILE%
REM Do not exit as this prevents the test run reporting correctly
GOTO end

:end
call %FUNCTIONS% :stopOSPL

