@ECHO OFF

MD bld
CD bld
MD idlpp
CD idlpp

ECHO "Compiling IDL with SPLICE IDL compiler"
idlpp -l java -C ../../Chat.idl

ECHO "Compiling IDL with JacORB IDL compiler"
java -classpath %JACORB_HOME%/lib/idl.jar:%JACORB_HOME%/lib/logkit-1.2.jar:%CLASSPATH% org.jacorb.idl.parser ../../Chat.idl

ECHO "Compiling generated java code"
javac -classpath ".;%OSPL_HOME%/jar/dcpscj.jar" -d .. Chat\*.java
IF NOT ERRORLEVEL==0 (
  ECHO:
  ECHO Compilation failed
  ECHO:
  GOTO error
)

CD ..

ECHO "Compiling application java code"
javac -classpath ".;%OSPL_HOME%/jar/dcpscj.jar" -d .  ..\chatroom\*.java
IF NOT ERRORLEVEL==0 (
  ECHO:
  ECHO Compilation failed
  ECHO:
  GOTO error
)

CD ..
GOTO end

:error
ECHO An error occurred, exiting now
:end
