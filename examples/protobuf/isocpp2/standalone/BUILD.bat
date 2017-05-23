@ECHO off

ECHO Building protobuf example (isocpp2)...

IF "%OSPL_HOME%"=="" (
    ECHO OSPL_HOME is NOT defined
    EXIT 1
)
IF "%PROTOBUF_HOME%"=="" (
    ECHO PROTOBUF_HOME is NOT defined. It must point to the protobuf installation directory
    EXIT 1
)

IF "%PROTOBUF_LIB_HOME%"=="" (
    ECHO PROTOBUF_LIB_HOME is NOT defined. It must point to the protobuf lib directory
    EXIT 1
)

protoc.exe --version
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
ECHO - OSPL_HOME is "%OSPL_HOME%"
ECHO - PROTOBUF_HOME is "%PROTOBUF_HOME%"
ECHO - PROTOBUF_LIB_HOME is %PROTOBUF_LIB_HOME%"
set MDFLAG=MD
echo %PROTOBUF_LIB_HOME%|find "debug" >nul
if %errorlevel% == 0 (set MDFLAG=MDd)
ECHO MD flag set to %MDFLAG%
ECHO Compiling proto files...

ECHO  - ../proto/address.proto
protoc.exe --cpp_out=.\generated --ddscpp_out=.\generated --proto_path="%OSPL_HOME%\examples\protobuf\proto" --proto_path="%PROTOBUF_HOME%\src" --proto_path="%OSPL_HOME%\include\protobuf" "%OSPL_HOME%\examples\protobuf\proto\address.proto"

ECHO  - %PROTOBUF_HOME%\src\google\protobuf\descriptor.proto
protoc.exe --cpp_out=.\generated --proto_path="%PROTOBUF_HOME%\src" "%PROTOBUF_HOME%\src\google\protobuf\descriptor.proto"

ECHO  - %OSPL_HOME%\src\tools\protobuf\protos\omg\dds\descriptor.proto
protoc.exe --cpp_out=.\generated --proto_path="%PROTOBUF_HOME%\src" --proto_path="%OSPL_HOME%\include\protobuf" "%OSPL_HOME%\include\protobuf\omg\dds\descriptor.proto"

REM need to rename this because the vs compiler flattens directory structure so this conflicts with the google descriptor
rename "%CD%\generated\omg\dds\descriptor.pb.cc" "descriptor1.pb.cc"

ECHO Compiling proto files done
ECHO Compiling cpp files...

ECHO Compiling publisher...
REM cl -DNEDEBUG -EHsc -TP -MD -nologo -incremental:no -subsystem:console -I"%PROTOBUF_HOME%\src\google\protobuf" -I.\generated -I"%OSPL_HOME%\include\dcps\C++\SACPP" -I"%OSPL_HOME%\include\dcps\C++\isocpp2" -I"%OSPL_HOME%\include" -I"%OSPL_HOME%\include\sys" -I..\..\.. ..\src\implementation.cpp ..\src\publisher.cpp .\generated\address.cpp .\generated\address.pb.cc .\generated\addressSplDcps.cpp  .\generated\ospl_protobuf_commonSplDcps.cpp .\generated\google\protobuf\descriptor.pb.cc .\generated\omg\dds\descriptor.pb.cc -L"%OSPL_HOME%\lib" -L"%PROTOBUF_LIB_HOME%" -ldcpsisocpp2 -lddskernel -lprotobuf -o publisher
cl /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_CRT_NONSTDC_NO_WARNINGS" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /EHsc /TP /%MDFLAG% /nologo /I "%PROTOBUF_HOME%\src" /I .\generated /I "%OSPL_HOME%\include\dcps\C++\SACPP" /I "%OSPL_HOME%\include\dcps\C++\isocpp2" /I "%OSPL_HOME%\include" /I "%OSPL_HOME%\include\sys" /I ..\..\.. ..\src\implementation.cpp ..\src\publisher.cpp .\generated\address.cpp .\generated\address.pb.cc .\generated\addressSplDcps.cpp  .\generated\ospl_protobuf_commonSplDcps.cpp .\generated\google\protobuf\descriptor.pb.cc .\generated\omg\dds\descriptor1.pb.cc /link "dcpsisocpp2.lib" "ddskernel.lib" "libprotobuf.lib" /nologo /incremental:no /subsystem:console /LIBPATH:"%OSPL_HOME%\lib" /LIBPATH:"%PROTOBUF_LIB_HOME%" /out:publisher.exe
ECHO Compiling publisher done

ECHO Compiling subscriber...
REM cl -DNEDEBUG -EHsc -TP -MD -nologo -incremental:no -subsystem:console -I"%PROTOBUF_HOME%\src\google\protobuf" -I.\generated -I"%OSPL_HOME%\include\dcps\C++\SACPP" -I"%OSPL_HOME%\include\dcps\C++\isocpp2" -I"%OSPL_HOME%\include" -I"%OSPL_HOME%\include\sys" -I..\..\.. ..\src\implementation.cpp ..\src\subscriber.cpp .\generated\address.cpp .\generated\address.pb.cc .\generated\addressSplDcps.cpp .\generated\ospl_protobuf_commonSplDcps.cpp .\generated\google\protobuf\descriptor.pb.cc .\generated\omg\dds\descriptor.pb.cc -L"%OSPL_HOME%\lib" -L"%PROTOBUF_LIB_HOME%" -ldcpsisocpp2 -lddskernel -lprotobuf -o subscriber
cl /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_CRT_NONSTDC_NO_WARNINGS" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /EHsc /TP /%MDFLAG% /nologo /I "%PROTOBUF_HOME%\src" /I .\generated /I "%OSPL_HOME%\include\dcps\C++\SACPP" /I "%OSPL_HOME%\include\dcps\C++\isocpp2" /I "%OSPL_HOME%\include" /I "%OSPL_HOME%\include\sys" /I ..\..\.. ..\src\implementation.cpp ..\src\subscriber.cpp .\generated\address.cpp .\generated\address.pb.cc .\generated\addressSplDcps.cpp  .\generated\ospl_protobuf_commonSplDcps.cpp .\generated\google\protobuf\descriptor.pb.cc .\generated\omg\dds\descriptor1.pb.cc /link "dcpsisocpp2.lib" "ddskernel.lib" "libprotobuf.lib" /nologo /incremental:no /subsystem:console /LIBPATH:"%OSPL_HOME%\lib" /LIBPATH:"%PROTOBUF_LIB_HOME%" /out:subscriber.exe
ECHO Compiling subscriber done
