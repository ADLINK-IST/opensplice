@ECHO OFF
SETLOCAL
echo "==== Calling check_osplhome===="
call %FUNCTIONS% :check_osplhome

echo "====Calling stopOSPL ===="
call %FUNCTIONS% :stopOSPL

set BLOKSIZE=100
set BLOKCOUNT=100

echo "==== Calling startOSPL ===="
call %FUNCTIONS% :startOSPL

ECHO Starting pong
set LEVEL=Starting pong
start java -classpath "classes;%OSPL_HOME%/jar/dcpssaj.jar" pong PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with m
set LEVEL=Starting ping with m
java -classpath "classes;%OSPL_HOME%/jar/dcpssaj.jar" ping %BLOKCOUNT% %BLOKSIZE% m PongRead PongWrite >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with q
set LEVEL=Starting ping with q
java -classpath "classes;%OSPL_HOME%/jar/dcpssaj.jar" ping %BLOKCOUNT% %BLOKSIZE% q PongRead PongWrite >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with s
set LEVEL=Starting ping with s
java -classpath "classes;%OSPL_HOME%/jar/dcpssaj.jar" ping %BLOKCOUNT% %BLOKSIZE% s PongRead PongWrite >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with f
set LEVEL=Starting ping with f
java -classpath "classes;%OSPL_HOME%/jar/dcpssaj.jar" ping %BLOKCOUNT% %BLOKSIZE% f PongRead PongWrite >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with t
set LEVEL=Starting ping with t
java -classpath "classes;%OSPL_HOME%/jar/dcpssaj.jar" ping 1 10 t PongRead PongWrite  >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 GOTO error

GOTO end

:error
ECHO An error occurred %LEVEL%, exiting example ... >> %LOGFILE%
REM Do not exit as this prevents the test run reporting correctly
GOTO end

:end
echo "===== calling stopOSPL ===="
call %FUNCTIONS% :stopOSPL
