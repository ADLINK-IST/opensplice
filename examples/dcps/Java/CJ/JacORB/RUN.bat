ospl start

set BLOKSIZE=100
set BLOKCOUNT=100

set SLEEP2=@ping -n 2 localhost
set SLEEP4=@ping -n 4 localhost
set SLEEP10=@ping -n 10 localhost

CD bld

%SLEEP10% >NUL

start java -classpath ".;%OSPL_HOME%/jar/dcpscj.jar" pong PongRead PongWrite

%SLEEP4% >NUL

java -classpath ".;%OSPL_HOME%/jar/dcpscj.jar" ping %BLOKCOUNT% %BLOKSIZE% m PongRead PongWrite

%SLEEP2% >NUL

java -classpath ".;%OSPL_HOME%/jar/dcpscj.jar" ping %BLOKCOUNT% %BLOKSIZE% q PongRead PongWrite

%SLEEP2% >NUL

java -classpath ".;%OSPL_HOME%/jar/dcpscj.jar" ping %BLOKCOUNT% %BLOKSIZE% s PongRead PongWrite

%SLEEP2% >NUL

java -classpath ".;%OSPL_HOME%/jar/dcpscj.jar" ping %BLOKCOUNT% %BLOKSIZE% f PongRead PongWrite

%SLEEP2% >NUL

java -classpath ".;%OSPL_HOME%/jar/dcpscj.jar" ping 1 10 t PongRead PongWrite

%SLEEP4% >NUL

CD ..

ospl stop

%SLEEP10% >NUL
