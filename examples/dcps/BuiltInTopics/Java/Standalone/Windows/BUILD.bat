@ECHO OFF
mkdir  bld
mkdir  exec
cd bld
mkdir classes

REM
REM Compile application java code
REM
echo "Compile application Java classes"

javac -classpath .;idlpp;"%OSPL_HOME%/jar/dcpssaj.jar" -d ./classes ../../../src/*.java
IF NOT ERRORLEVEL==0 (
  ECHO:
  ECHO *** Compilation of application Java classes failed
  ECHO:
  GOTO error
)
REM
REM Create jar files
REM
echo "Create jar files"
cd ../exec
cd
jar cf BuildInTopicsDataSubscriber.jar -C ../bld/classes . 

cd ..
GOTO end

:error
ECHO An error occurred, exiting now
:end
