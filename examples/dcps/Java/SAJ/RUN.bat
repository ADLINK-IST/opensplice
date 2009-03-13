set BLOKSIZE=100
set BLOKCOUNT=100

set SLEEP2=@ping -n 2 localhost
set SLEEP4=@ping -n 4 localhost

CD bld

ospl start

%SLEEP4% >NUL

start java -classpath ".;%OSPL_HOME%/jar/dcpssaj.jar" pong PongRead PongWrite

%SLEEP4% >NUL

java -classpath ".;%OSPL_HOME%/jar/dcpssaj.jar" ping %BLOKCOUNT% %BLOKSIZE% m PongRead PongWrite

%SLEEP2% >NUL

java -classpath ".;%OSPL_HOME%/jar/dcpssaj.jar" ping %BLOKCOUNT% %BLOKSIZE% q PongRead PongWrite

%SLEEP2% >NUL

java -classpath ".;%OSPL_HOME%/jar/dcpssaj.jar" ping %BLOKCOUNT% %BLOKSIZE% s PongRead PongWrite

%SLEEP2% >NUL

java -classpath ".;%OSPL_HOME%/jar/dcpssaj.jar" ping %BLOKCOUNT% %BLOKSIZE% f PongRead PongWrite

%SLEEP2% >NUL

java -classpath ".;%OSPL_HOME%/jar/dcpssaj.jar" ping 1 10 t PongRead PongWrite 

%SLEEP4% >NUL

CD ..

ospl stop
