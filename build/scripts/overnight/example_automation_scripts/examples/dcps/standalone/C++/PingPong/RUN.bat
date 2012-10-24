@echo off

set SLEEP4=@C:\WINDOWS\system32\ping.exe -n 4 localhost

ospl start
set LEVEL=Starting ospl
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting ospl %ERRORLEVEL%

%SLEEP4% >NUL

set PATH=%~dp0\Ping\Release;%~dp0\Ping\Debug;%PATH%
set PATH=%~dp0\Pong\Release;%~dp0\Pong\Debug;%PATH%

set BLOKSIZE=100
set BLOKCOUNT=100

ECHO Starting ospl
ospl start
set LEVEL=Starting ospl
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting ospl %ERRORLEVEL%

%SLEEP4% >NUL

ECHO Starting pong
set LEVEL=Starting pong
start pong.exe PongRead PongWrite
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting pong %ERRORLEVEL%

%SLEEP4% >NUL

ECHO Starting ping with m
set LEVEL=Starting ping with m
ping.exe %BLOKCOUNT% %BLOKSIZE% m PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with q
set LEVEL=Starting ping with q
ping.exe %BLOKCOUNT% %BLOKSIZE% q PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with s
set LEVEL=Starting ping with s
ping.exe %BLOKCOUNT% %BLOKSIZE% s PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with f
set LEVEL=Starting ping with f
ping.exe %BLOKCOUNT% %BLOKSIZE% f PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with t
set LEVEL=Starting ping with t
ping.exe 1  10 t PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error


GOTO end

:error
ECHO An error occurred %LEVEL%, exiting example ...
REM Do not exit as this prevents the test run reporting correctly
GOTO end

:end

ospl stop
if %ERRORLEVEL% NEQ 0 ECHO An error occurred stopping ospl

%SLEEP4% >NUL
