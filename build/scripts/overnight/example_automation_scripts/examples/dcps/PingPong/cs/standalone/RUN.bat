@ECHO OFF

if not exist "sacs_pingpong_pong.exe" (
   echo "Error: The pong executable does not exist - aborting example" >> %LOGFILE%
   goto:END
   )

if not exist "sacs_pingpong_ping.exe" (
   echo "Error: The ping executable does not exist - aborting example" >> %LOGFILE%
   goto:END
   )
 
set BLOKSIZE=100
set BLOKCOUNT=100
   
call %FUNCTIONS% :check_osplhome

call %FUNCTIONS% :startOSPL

ECHO "Starting Pong"
set LEVEL=Starting pong
start CMD /C sacs_pingpong_pong.exe PongRead PongWrite ^> pong.log
if %ERRORLEVEL% NEQ 0 GOTO PingPongErr

%SLEEP4% >NUL

ECHO "Starting ping with m"
set LEVEL=Starting ping with m
sacs_pingpong_ping.exe %BLOKCOUNT% %BLOKSIZE% m PongRead PongWrite >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 GOTO PingPongErr

%SLEEP4% >NUL

ECHO "Starting ping with q"
set LEVEL=Starting ping with q
sacs_pingpong_ping.exe %BLOKCOUNT% %BLOKSIZE% q PongRead PongWrite >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 GOTO PingPongErr

%SLEEP4% >NUL

ECHO "Starting ping with s"
set LEVEL=Starting ping with s
sacs_pingpong_ping.exe %BLOKCOUNT% %BLOKSIZE% s PongRead PongWrite >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 GOTO PingPongErr

%SLEEP4% >NUL

ECHO "Starting ping with f"
set LEVEL=Starting ping with f
sacs_pingpong_ping.exe %BLOKCOUNT% %BLOKSIZE% f PongRead PongWrite >> %LOGFILE%
if %ERRORLEVEL% NEQ 0 GOTO PingPongErr

%SLEEP4% >NUL

ECHO "Starting ping with t"
set LEVEL=Starting ping with t
sacs_pingpong_ping.exe 1  10 t PongRead PongWrite >> %LOGFILE%   
if %ERRORLEVEL% NEQ 0 GOTO PingPongErr
goto:END

:PingPongErr
   ECHO "An error occurred %LEVEL%  ....." >> %LOGFILE%
   goto:END


:END
   call %FUNCTIONS% :stopOSPL