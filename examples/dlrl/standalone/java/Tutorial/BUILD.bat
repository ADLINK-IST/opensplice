@ECHO OFF

IF NOT DEFINED OSPL_HOME (
  ECHO:
  ECHO The OpenSplice environment has not been SET properly.
  ECHO Please run the OpenSplice release.bat file
  ECHO:
  GOTO end
)

MD bld\idlpp

REM
REM Generate java classes from DCPS IDL
REM

ECHO "Compiling DCPS IDL with SPLICE IDL compiler"
idlpp -S -l java -d bld\idlpp data_files\ExtChat.idl

REM
REM Compile generated java code
REM
ECHO "Compiling generated Java classes"
javac -classpath "%OSPL_HOME%\jar\dcpssaj.jar" -d bld -sourcepath bld\idlpp bld\idlpp\Chat\*.java
IF NOT ERRORLEVEL==0 (
  ECHO:
  ECHO Compilation failed
  ECHO:
  GOTO error
)

REM
REM Generate java classes from DLRL IDL and XML mapping file for WhiteListedMessageboard
REM
ECHO "Compiling DLRL code for WhiteListedMessageBoard application"
MD bld\ospldcg\WhiteListedMessageBoard

call ospldcg -dcps data_files\ExtChat.idl -dlrl data_files\WhiteListObjects.idl -mapping data_files\mapping.xml -l JAVA -o bld\ospldcg\WhiteListedMessageBoard

REM
REM Generate java classes from DLRL IDL and XML mapping file for WhiteListEditor and WhiteListViewer
REM
ECHO "Compiling DLRL code for WhiteListEditor and WhiteListViewer applications"
MD bld\ospldcg\WhiteListEditor
call ospldcg -dcps data_files\ExtChat.idl -dlrl data_files\WhiteListObjects_editor.idl -mapping data_files\mapping_editor.xml -l JAVA -o bld\ospldcg\WhiteListEditor

REM
REM Compile generated java code
REM
MD bld\WhiteListedMessageBoard
MD bld\WhiteListEditor
ECHO "Compiling generated Java classes"
javac -classpath "bld;%OSPL_HOME%\jar\dcpssaj.jar;%OSPL_HOME%\jar\dlrlsaj.jar" -d bld\WhiteListedMessageBoard -sourcepath bld\ospldcg\WhiteListedMessageBoard bld\ospldcg\WhiteListedMessageBoard\DLRLChat\*.java
IF NOT ERRORLEVEL==0 (
  ECHO:
  ECHO Compilation failed
  ECHO:
  GOTO error
)
javac -classpath "bld;%OSPL_HOME%\jar\dcpssaj.jar;%OSPL_HOME%\jar\dlrlsaj.jar" -d bld\WhiteListEditor -sourcepath bld\ospldcg\WhiteListEditor bld\ospldcg\WhiteListEditor\DLRLChat\*.java
IF NOT ERRORLEVEL==0 (
  ECHO:
  ECHO Compilation failed
  ECHO:
  GOTO error
)

REM
REM Compile application java code
REM
ECHO "Compiling application Java classes"
javac -classpath "%OSPL_HOME%\jar\dcpssaj.jar;%OSPL_HOME%\jar\dlrlsaj.jar;bld" -d bld Common\*.java
IF NOT ERRORLEVEL==0 (
  ECHO:
  ECHO Compilation failed
  ECHO:
  GOTO error
)
javac -classpath "%OSPL_HOME%\jar\dcpssaj.jar;%OSPL_HOME%\jar\dlrlsaj.jar;bld;bld\WhiteListedMessageBoard" -d bld WhiteListedMessageBoard\*.java
IF NOT ERRORLEVEL==0 (
  ECHO:
  ECHO Compilation failed
  ECHO:
  GOTO error
)
javac -classpath "%OSPL_HOME%\jar\dcpssaj.jar;%OSPL_HOME%\jar\dlrlsaj.jar;bld;bld\WhiteListEditor" -d bld WhiteListViewer\*.java
IF NOT ERRORLEVEL==0 (
  ECHO:
  ECHO Compilation failed
  ECHO:
  GOTO error
)
javac -classpath "%OSPL_HOME%\jar\dcpssaj.jar;%OSPL_HOME%\jar\dlrlsaj.jar;bld;bld\WhiteListEditor" -d bld WhiteListEditor\*.java
IF NOT ERRORLEVEL==0 (
  ECHO:
  ECHO Compilation failed
  ECHO:
  GOTO error
)

GOTO end

:error
ECHO An error occurred, exiting now
:end
