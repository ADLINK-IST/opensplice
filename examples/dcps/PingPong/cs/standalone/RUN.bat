@ECHO OFF
set SLEEP4=@C:\WINDOWS\system32\ping.exe -n 4 localhost

set BLOKSIZE=100
set BLOKCOUNT=100

ECHO Starting Pong
set LEVEL=Starting pong
start "" /B sacs_pingpong_pong.exe PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with m
set LEVEL=Starting ping with m
sacs_pingpong_ping.exe %BLOKCOUNT% %BLOKSIZE% m PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with q
set LEVEL=Starting ping with q
sacs_pingpong_ping.exe %BLOKCOUNT% %BLOKSIZE% q PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Startinf ping with s
set LEVEL=Starting ping with s
sacs_pingpong_ping.exe %BLOKCOUNT% %BLOKSIZE% s PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with f
set LEVEL=Starting ping with f
sacs_pingpong_ping.exe %BLOKCOUNT% %BLOKSIZE% f PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with a
set LEVEL=Starting ping with a
sacs_pingpong_ping.exe %BLOKCOUNT% %BLOKSIZE% a PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with t
set LEVEL=Starting ping with t
sacs_pingpong_ping.exe 1  10 t PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

GOTO end

:error
ECHO An error occurred %LEVEL%, example .....
GOTO end

:end