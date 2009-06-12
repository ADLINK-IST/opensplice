@echo off

set PATH=%~dp0\Ping\Release;%~dp0\Ping\Debug;%PATH%
set PATH=%~dp0\Pong\Release;%~dp0\Pong\Debug;%PATH%

set BLOKSIZE=100
set BLOKCOUNT=100

set SLEEP2=@ping -n 2 localhost
set SLEEP4=@ping -n 4 localhost

ospl start

%SLEEP4% >NUL

start pong.exe PongRead PongWrite

%SLEEP4% >NUL

ping.exe %BLOKCOUNT% %BLOKSIZE% m PongRead PongWrite

%SLEEP2% >NUL

ping.exe %BLOKCOUNT% %BLOKSIZE% q PongRead PongWrite

%SLEEP2% >NUL

ping.exe %BLOKCOUNT% %BLOKSIZE% s PongRead PongWrite

%SLEEP2% >NUL

ping.exe %BLOKCOUNT% %BLOKSIZE% f PongRead PongWrite

%SLEEP2% >NUL

ping.exe 1  10 t PongRead PongWrite

%SLEEP4% >NUL

ospl stop
