@ECHO OFF
SETLOCAL

set SLEEP4=@C:\WINDOWS\system32\ping.exe -n 4 localhost

set BLOKSIZE=100
set BLOKCOUNT=100

ECHO Starting pong
set LEVEL=Starting pong
start "" /B java -Djava.endorsed.dirs="%JACORB_HOME%/lib/endorsed" -classpath ".;classes;%OSPL_HOME%/jar/dcpscj.jar" pong PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

%SLEEP4% >NUL

ECHO Starting ping with m
set LEVEL=Starting ping with m
java -Djava.endorsed.dirs="%JACORB_HOME%/lib/endorsed" -classpath ".;classes;%OSPL_HOME%/jar/dcpscj.jar" ping %BLOKCOUNT% %BLOKSIZE% m PongRead PongWrite
if %ERRORLEVEL% NEQ 0 ECHO Error %LEVEL%

%SLEEP4% >NUL

ECHO Starting ping with q
set LEVEL=Starting ping with q
java -Djava.endorsed.dirs="%JACORB_HOME%\lib\endorsed" -classpath ".;classes;%OSPL_HOME%/jar/dcpscj.jar" ping %BLOKCOUNT% %BLOKSIZE% q PongRead PongWrite
if %ERRORLEVEL% NEQ 0 ECHO Error %LEVEL%

%SLEEP4% >NUL

ECHO Starting ping with s
set LEVEL=Starting ping with s
java -Djava.endorsed.dirs="%JACORB_HOME%\lib\endorsed" -classpath ".;classes;%OSPL_HOME%/jar/dcpscj.jar" ping %BLOKCOUNT% %BLOKSIZE% s PongRead PongWrite
if %ERRORLEVEL% NEQ 0 ECHO Error %LEVEL%

%SLEEP4% >NUL

ECHO Starting ping with f
set LEVEL=Starting ping with f
java -Djava.endorsed.dirs="%JACORB_HOME%\lib\endorsed" -classpath ".;classes;%OSPL_HOME%/jar/dcpscj.jar" ping %BLOKCOUNT% %BLOKSIZE% f PongRead PongWrite
if %ERRORLEVEL% NEQ 0 ECHO Error %LEVEL%

%SLEEP4% >NUL 

ECHO Starting ping with a
set LEVEL=Starting ping with a
java -Djava.endorsed.dirs="%JACORB_HOME%\lib\endorsed" -classpath ".;classes;%OSPL_HOME%/jar/dcpscj.jar" ping %BLOKCOUNT% %BLOKSIZE% a PongRead PongWrite
if %ERRORLEVEL% NEQ 0 ECHO Error %LEVEL%

%SLEEP4% >NUL 

ECHO Starting ping with t
set LEVEL=Starting ping with t
java -Djava.endorsed.dirs="%JACORB_HOME%\lib\endorsed" -classpath ".;classes;%OSPL_HOME%/jar/dcpscj.jar" ping 1 10 t PongRead PongWrite
if %ERRORLEVEL% NEQ 0 GOTO error

GOTO end

:error
ECHO An error occurred %LEVEL%, exiting example .....
GOTO end

:end