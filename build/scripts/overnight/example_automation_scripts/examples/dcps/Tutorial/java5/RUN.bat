SETLOCAL
@ECHO OFF

call %FUNCTIONS% :stopOSPL

call %FUNCTIONS% :startOSPL

ECHO Starting MessageBoard
SET LEVEL=Starting MessageBoard
start CMD /C java -jar messageboard/java5_Messageboard.jar ^> messageBoard.log
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting UserLoad
SET LEVEL=Starting UserLoad
start CMD /C java -jar userload/java5_UserLoad.jar ^> userLoad.log
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting UserLoad %ERRORLEVEL%

%SLEEP4% >NUL

ECHO Starting Chatter
SET LEVEL=Starting Chatter
java -jar chatter/java5_Chatter.jar >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting Chatter with terminate message>> %LOGFILE%
java -jar chatter/java5_Chatter.jar -1 >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 GOTO error

GOTO end

:error
ECHO An error occurred %LEVEL%, exiting example ...
REM Do not exit as this prevents the test run reporting correctly
GOTO end

:end
call %FUNCTIONS% :stopOSPL