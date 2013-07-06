@ECHO OFF

call %FUNCTIONS% :deleteDBFFiles

call %FUNCTIONS% :startOSPL

sleep 4

ECHO RUN MESSAGEBOARD
start CMD /C java -classpath "bld;bld\WhiteListedMessageBoard;%OSPL_HOME%\jar\dcpssaj.jar;%OSPL_HOME%\jar\dlrlsaj.jar" WhiteListedMessageBoard.WhiteListedMessageBoard -name family ^> messageBoard.log
if %ERRORLEVEL% NEQ 0 GOTO error 

sleep 4

ECHO RUN EDITOR
start CMD /C java -classpath "bld;bld\WhiteListEditor;%OSPL_HOME%\jar\dcpssaj.jar;%OSPL_HOME%\jar\dlrlsaj.jar" WhiteListEditor.WhiteListEditor -name family -add 1,mam 2,dad 3,John ^> editor.log
if %ERRORLEVEL% NEQ 0 ECHO An error occurred running the Editor

sleep 4

ECHO RUN VIEWER
start CMD /C java -classpath "bld;bld\WhiteListEditor;%OSPL_HOME%\jar\dcpssaj.jar;%OSPL_HOME%\jar\dlrlsaj.jar" WhiteListViewer.WhiteListViewer ^> viewer.log

sleep 4

ECHO start Chatter with valid parameters
call "%OSPL_HOME%\examples\dcps\Tutorial\cpp\standalone\Chatter.exe" 3 John >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 ECHO an error occurred sending a message from a valid user

sleep 4


ECHO start Chatter with invalid parameters
call "%OSPL_HOME%\examples\dcps\Tutorial\cpp\standalone\Chatter.exe" 4 Dave >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 ECHO an error occurred sending a message from an invalid user

sleep 4


ECHO start Chatter with terminate message
call "%OSPL_HOME%\examples\dcps\Tutorial\cpp\standalone\Chatter.exe" -1 >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 ECHO an error occurred sending termination message
IF "%EXRUNTYPE%"=="shm" GOTO osplstop

GOTO end

sleep 4

:error
ECHO An error "%ERRORLEVEL%" occurred, exiting example ... >> %LOGFILE%
IF "%EXRUNTYPE%"=="shm" GOTO osplstop
GOTO end

:osplstop
   call %FUNCTIONS% :stopOSPL

:end
