@echo OFF
if "%OSPL_HOME%" == "" (
    echo "*** Please source release.bat to set OpenSplice environment variables";
    echo "!!! ERROR OpenSplice environment variables NOT SET" >> SubResult.txt
    exit;
)
echo OSPL_HOME is %OSPL_HOME%
echo OSPL_URI is %OSPL_URI%

rem setting variables 
SET SLEEP2=ping 127.0.0.1 -n 2
SET SLEEP5=ping 127.0.0.1 -n 5
@echo off

rem checking DBF Files
%VG_OSPL_START% ospl stop
%SLEEP2% >NUL
echo.
echo "=== checking DBF Files"
if exist "%TMP%\osp*" (
echo "*** .DBF files exist ***
dir "%TMP%\osp*"
echo "=== deleting "%TMP%\osp*"
del /F /Q "%TMP%\osp*"
)

%VG_OSPL_START% ospl start
REM %VG_START_SLEEP%
echo "=== Launching Listener "
call startPublisher.bat
call startSubscriber.bat subResult.txt
%SLEEP2% >NUL
echo "=== Checking  Listener results"
rem Checking result of subscriber
FIND /C /I "message received" subResult.txt  > nul
if NOT %ERRORLEVEL%==0 GOTO Err
FIND /C /I "userID" subResult.txt  > nul
if NOT %ERRORLEVEL%==0 GOTO Err
FIND /C /I "Message :" subResult.txt  > nul
echo OK
GOTO End
:Err
echo "*** ERROR : example Listener failed ";
:End
rem Don't kill it too soon.
%SLEEP2% >NUL
%VG_OSPL_START% ospl stop
%SLEEP2% >NUL
