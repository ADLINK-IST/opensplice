@ECHO OFF
set SLEEP4=@C:\WINDOWS\system32\ping.exe -n 4 localhost

ECHO Starting ospl
set LEVEL=Starting ospl
ospl start
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting ospl %ERRORLEVEL%

%SLEEP4% >NUL

set BLOKSIZE=100
set BLOKCOUNT=100

ECHO Starting Pong
set LEVEL=Starting pong
start Pong\pong.exe PongRead PongWrite
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting pong %ERRORLEVEL%

%SLEEP4% >NUL

ECHO Starting ping with m
set LEVEL=Starting ping with m
Ping\ping.exe %BLOKCOUNT% %BLOKSIZE% m PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with q
set LEVEL=Starting ping with q
Ping\ping.exe %BLOKCOUNT% %BLOKSIZE% q PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Startinf ping with s
set LEVEL=Starting ping with s
Ping\ping.exe %BLOKCOUNT% %BLOKSIZE% s PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with f
set LEVEL=Starting ping with f
Ping\ping.exe %BLOKCOUNT% %BLOKSIZE% f PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with t
set LEVEL=Starting ping with t
Ping\ping.exe 1  10 t PongRead PongWrite 
if %ERRORLEVEL% NEQ 0 GOTO error

GOTO end

:error
ECHO An error occurred %LEVEL%, example .....
REM Do not exit as this prevents the test run reporting correctly
GOTO end

:end

ospl stop
if %ERRORLEVEL% NEQ 0 ECHO An error occurred stopping ospl

%SLEEP4% >NUL
