@ECHO off

ECHO Building protobuf example (java5)...

IF "%OSPL_HOME%"=="" (
    ECHO OSPL_HOME is NOT defined
    EXIT 1
)
IF "%PROTOBUF_HOME%"=="" (
    ECHO PROTOBUF_HOME is NOT defined. It must point to the protobuf installation directory
    EXIT 1
)

protoc --version
IF "%ERRORLEVEL%"=="1" (
   ECHO Problem running protoc - cannot build protobuf example...
   EXIT 1
)

IF NOT EXIST generated (
    mkdir generated 
)

IF NOT EXIST bld (
    mkdir bld
)
ECHO Compiling proto files...
ECHO  - ../proto/address.proto
ECHO - OSPL_HOME is "%OSPL_HOME%"
ECHO - PROTOBUF_HOME is "%PROTOBUF_HOME%"

protoc --java_out=.\generated --ddsjava_out=.\generated --proto_path="%OSPL_HOME%\examples\protobuf\proto" --proto_path="%PROTOBUF_HOME%\src" --proto_path="%OSPL_HOME%\include\protobuf" "%OSPL_HOME%\examples\protobuf\proto\address.proto"

ECHO Compiling proto files done
ECHO Compiling java files... 
SET CP_ARGS="%OSPL_HOME%\jar\dcpssaj5.jar;%OSPL_HOME%\jar\dcpsprotobuf.jar"

SET SP_ARGS=generated;src
SET XP_ARGS=-J-Xss2m

ECHO  - ..\src\*.java
javac %XP_ARGS% -cp %CP_ARGS% -sourcepath %SP_ARGS% -d bld ..\src\*.java

ECHO  - generated\address\*.java
javac %XP_ARGS% -cp %CP_ARGS% -sourcepath %SP_ARGS% -d bld generated\address\*.java

ECHO  - generated\address\dds\*.java
javac %XP_ARGS% -cp %CP_ARGS% -sourcepath %SP_ARGS% -d bld generated\address\dds\*.java

ECHO  - generated\org\omg\dds\protobuf\*.java
javac %XP_ARGS% -cp %CP_ARGS% -sourcepath %SP_ARGS% -d bld generated\org\omg\dds\protobuf\*.java

ECHO Compiling java files done
SET CP=Class-Path: ../../../../jar/dcpssaj5.jar ../../../../jar/dcpsprotobuf.jar

ECHO Creating publisher jar file... 
@ECHO Main-Class: ProtobufPublisher > publisher.mf
@ECHO %CP% >> publisher.mf
jar cmf publisher.mf saj5-protobuf-publisher.jar -C bld .
del publisher.mf
ECHO Done

ECHO Creating subscriber jar file... 
@ECHO Main-Class: ProtobufSubscriber > subscriber.mf
@ECHO %CP% >> subscriber.mf
jar cmf subscriber.mf saj5-protobuf-subscriber.jar -C bld .
del subscriber.mf
ECHO Done
