ospl start

set BLOKSIZE=100
set BLOKCOUNT=100

set SLEEP2=@ping -n 2 localhost
set SLEEP4=@ping -n 4 localhost
set SLEEP10=@ping -n 10 localhost

%SLEEP10% >NUL

start java -classpath "%OSPL_HOME%/jar/dcpssaj.jar;bld" pong PongRead PongWrite

%SLEEP4% >NUL

java -classpath "%OSPL_HOME%/jar/dcpssaj.jar;bld" ping %BLOKCOUNT% %BLOKSIZE% m PongRead PongWrite

%SLEEP2% >NUL

java -classpath "%OSPL_HOME%/jar/dcpssaj.jar;bld" ping %BLOKCOUNT% %BLOKSIZE% q PongRead PongWrite

%SLEEP2% >NUL

java -classpath "%OSPL_HOME%/jar/dcpssaj.jar;bld" ping %BLOKCOUNT% %BLOKSIZE% s PongRead PongWrite

%SLEEP2% >NUL

java -classpath "%OSPL_HOME%/jar/dcpssaj.jar;bld" ping %BLOKCOUNT% %BLOKSIZE% f PongRead PongWrite

%SLEEP2% >NUL

java -classpath "%OSPL_HOME%/jar/dcpssaj.jar;bld" ping 1 10 t PongRead PongWrite

%SLEEP4% >NUL

ospl stop

%SLEEP10% >NUL
