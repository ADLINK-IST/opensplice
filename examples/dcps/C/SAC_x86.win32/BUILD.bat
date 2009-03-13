@ECHO OFF

MD bld
CD bld


IF NOT DEFINED OSPL_HOME (
  ECHO:
  ECHO The OpenSplice environment has not been SET properly.
  ECHO Please run the OpenSplice release.bat file
  ECHO:
  GOTO end
)


REM ------------------------------------------------------------
REM
REM PLEASE READ THIS
REM
REM Fill the variables MSVC and PLATFORMSDK with the correct
REM paths in order to customize this script for your system.
REM
REM Examples:
REM
REM SET MSVC=C:\Program Files\Microsoft Visual Studio 8\VC
REM SET PLATFORMSDK=C:\Program Files\Microsoft Platform SDK
REM
REM ------------------------------------------------------------


IF not defined MSVC (
  ECHO:
  ECHO The MSVC environment variable has not been SET.
  ECHO Please edit the BUILD file to customize it to your system.
  ECHO:
  GOTO error
)

IF not defined PLATFORMSDK (
  ECHO:
  ECHO The PLATFORMSDK environment variable has not been SET.
  ECHO Please edit the BUILD file to customize it to your system.
  ECHO:
  GOTO error
)


REM Make sure cl and link can be found
SET PATH=%PATH%;%MSVC%\bin

REM Define the correct include paths for cl
SET INCLUDES=/I.
SET INCLUDES=%INCLUDES% /I"%OSPL_HOME%\include"
SET INCLUDES=%INCLUDES% /I"%OSPL_HOME%\include\sys"
SET INCLUDES=%INCLUDES% /I"%OSPL_HOME%\include\dcps\C\SAC"
SET INCLUDES=%INCLUDES% /I"%MSVC%\include"
SET INCLUDES=%INCLUDES% /I"%PLATFORMSDK%\include"

REM Define the correct options for link
SET LDLIBS=/LIBPATH:"%OSPL_HOME%\lib"
SET LDLIBS=%LDLIBS% /LIBPATH:"%MSVC%\lib"
SET LDLIBS=%LDLIBS% /LIBPATH:"%PLATFORMSDK%\lib"
SET LDLIBS=%LDLIBS% ddsdatabase.lib
SET LDLIBS=%LDLIBS% dcpssac.lib

ECHO "Compiling IDL with SPLICE IDL compiler"
idlpp -S -l c ../pingpong.idl


ECHO "Compiling SAC User Types support code"
cl %INCLUDES% /c pingpongSacDcps.c
IF NOT ERRORLEVEL==0 (
  ECHO:
  ECHO Compilation failed
  ECHO:
  GOTO error
)


ECHO "Compiling SPLICE User Types support code"
cl %INCLUDES% /c pingpongSplDcps.c
IF NOT ERRORLEVEL==0 (
  ECHO:
  ECHO Compilation failed
  ECHO:
  GOTO error
)

ECHO "Compiling PING"
cl %INCLUDES% /D_CRT_SECURE_NO_DEPRECATE /c ..\ping.c
IF NOT ERRORLEVEL==0 (
  ECHO:
  ECHO Compilation failed
  ECHO:
  GOTO error
)

ECHO "Compiling PONG"
cl %INCLUDES% /D_CRT_SECURE_NO_DEPRECATE /c ..\pong.c
IF NOT ERRORLEVEL==0 (
  ECHO:
  ECHO Compilation failed
  ECHO:
  GOTO error
)


ECHO "Linking PING"
link ping.obj pingpongSacDcps.obj pingpongSplDcps.obj %LDLIBS% /OUT:..\Ping\ping.exe
IF NOT ERRORLEVEL==0 (
  ECHO:
  ECHO Linking failed
  ECHO:
  GOTO error
)

ECHO "Linking PONG"
link pong.obj pingpongSacDcps.obj pingpongSplDcps.obj %LDLIBS% /OUT:..\Pong\pong.exe
IF NOT ERRORLEVEL==0 (
  ECHO:
  ECHO Linking failed
  ECHO:
  GOTO error
)

GOTO end

:error
ECHO An error occurred, exiting now
:end
CD ..
