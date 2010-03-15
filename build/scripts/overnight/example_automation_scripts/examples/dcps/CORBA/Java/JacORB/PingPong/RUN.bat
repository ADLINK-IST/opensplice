ECHO Starting ospl
set LEVEL=Starting ospl
ospl start
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting ospl %ERRORLEVEL%

sleep 4 

set BLOKSIZE=100
set BLOKCOUNT=100

CD bld

ECHO Starting pong
set LEVEL=Starting pong
start java -classpath ".;%OSPL_HOME%/jar/dcpscj.jar" pong PongRead PongWrite
if %ERRORLEVEL% NEQ 0 ECHO An error occurred starting pong %ERRORLEVEL%

sleep 4

ECHO Starting ping with m
set LEVEL=Starting ping with m
java -classpath ".;%OSPL_HOME%/jar/dcpscj.jar" ping %BLOKCOUNT% %BLOKSIZE% m PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

sleep 4

ECHO Starting ping with q
set LEVEL=Starting ping with q
java -classpath ".;%OSPL_HOME%/jar/dcpscj.jar" ping %BLOKCOUNT% %BLOKSIZE% q PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

sleep 4

ECHO Starting ping with s
set LEVEL=Starting ping with s
java -classpath ".;%OSPL_HOME%/jar/dcpscj.jar" ping %BLOKCOUNT% %BLOKSIZE% s PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

sleep 4

ECHO Starting ping with f
set LEVEL=Starting ping with f
java -classpath ".;%OSPL_HOME%/jar/dcpscj.jar" ping %BLOKCOUNT% %BLOKSIZE% f PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

sleep 4 

ECHO Starting ping with t
set LEVEL=Starting ping with t
java -classpath ".;%OSPL_HOME%/jar/dcpscj.jar" ping 1 10 t PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error


CD ..

GOTO end

:error
ECHO An error occurred %LEVEL%, exiting example ...
GOTO end

:end

ospl stop
if %ERRORLEVEL% NEQ 0 ECHO Error stopping ospl

sleep 4