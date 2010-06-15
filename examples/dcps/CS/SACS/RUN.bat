REM If you have built the examples in Debug configuration then you need to to use the alternate command
REM line provided to use the correct Debug executable and comment the Release executable.
REM By default the Release executable is called first.

ospl start

set BLOKSIZE=100
set BLOKCOUNT=100

set SLEEP2=@ping -n 2 localhost
set SLEEP4=@ping -n 4 localhost
set SLEEP10=@ping -n 10 localhost

%SLEEP10% >NUL

start Pong\Pong.exe PongRead PongWrite
REM start Pong\bin\Debug\Pong.exe PongRead PongWrite

%SLEEP4% >NUL

Ping\Ping.exe %BLOKCOUNT% %BLOKSIZE% m PongRead PongWrite
REM Ping\bin\Debug\Ping.exe %BLOKCOUNT% %BLOKSIZE% m PongRead PongWrite

%SLEEP2% >NUL

Ping\Ping.exe %BLOKCOUNT% %BLOKSIZE% s PongRead PongWrite
REM Ping\bin\Debug\Ping.exe %BLOKCOUNT% %BLOKSIZE% s PongRead PongWrite

%SLEEP2% >NUL

Ping\Ping.exe %BLOKCOUNT% %BLOKSIZE% f PongRead PongWrite
REM Ping\bin\Debug\Ping.exe %BLOKCOUNT% %BLOKSIZE% f PongRead PongWrite

%SLEEP2% >NUL

Ping\Ping.exe 1  10 t PongRead PongWrite 
REM Ping\bin\Debug\Ping.exe 1  10 t PongRead PongWrite 

%SLEEP4% >NUL

ospl stop

%SLEEP10% >NUL
