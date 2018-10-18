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
start java -jar pong/java5_pong.jar PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with m
set LEVEL=Starting ping with m
java -jar ping/java5_ping.jar %BLOKCOUNT% %BLOKSIZE% m PongRead PongWrite >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with q
set LEVEL=Starting ping with q
java -jar ping/java5_ping.jar %BLOKCOUNT% %BLOKSIZE% q PongRead PongWrite >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with s
set LEVEL=Starting ping with s
java -jar ping/java5_ping.jar %BLOKCOUNT% %BLOKSIZE% s PongRead PongWrite >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with f
set LEVEL=Starting ping with f
java -jar ping/java5_ping.jar %BLOKCOUNT% %BLOKSIZE% f PongRead PongWrite >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with t
set LEVEL=Starting ping with t
java -jar ping/java5_ping.jar 1 10 t PongRead PongWrite  >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 GOTO error

GOTO end

:error
ECHO An error occurred %LEVEL%, exiting example ... >> %LOGFILE%
REM Do not exit as this prevents the test run reporting correctly
GOTO end

:end
echo "===== calling stopOSPL ===="
call %FUNCTIONS% :stopOSPL
