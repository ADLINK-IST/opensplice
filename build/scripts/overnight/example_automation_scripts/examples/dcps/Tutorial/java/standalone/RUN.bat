SETLOCAL
@ECHO OFF

call %FUNCTIONS% :stopOSPL

call %FUNCTIONS% :startOSPL

ECHO Starting MessageBoard
SET LEVEL=Starting MessageBoard
start CMD /C java -classpath "%OSPL_HOME%\jar\dcpssaj.jar;classes" chatroom.MessageBoard ^> messageBoard.log
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting UserLoad
SET LEVEL=Starting UserLoad
start CMD /C java -classpath "%OSPL_HOME%\jar\dcpssaj.jar;classes" chatroom.UserLoad ^> userLoad.log
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting UserLoad %ERRORLEVEL%

%SLEEP4% >NUL

ECHO Starting Chatter
SET LEVEL=Starting Chatter
java -classpath "%OSPL_HOME%\jar\dcpssaj.jar;classes" chatroom.Chatter >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting Chatter with terminate message>> %LOGFILE%
java -classpath "%OSPL_HOME%\jar\dcpssaj.jar;classes" chatroom.Chatter -1 >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 GOTO error

GOTO end

:error
ECHO An error occurred %LEVEL%, exiting example ...
REM Do not exit as this prevents the test run reporting correctly
GOTO end

:end
call %FUNCTIONS% :stopOSPL