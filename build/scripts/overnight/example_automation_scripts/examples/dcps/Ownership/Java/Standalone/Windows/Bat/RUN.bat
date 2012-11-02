@echo OFF
if "%OSPL_HOME%" == "" (
    echo "*** Please source release.com to set OpenSplice environment variables";
    echo "!!! ERROR OpenSplice environment variables NOT SET" >> SubResult.txt
    exit;
)
echo OSPL_HOME is %OSPL_HOME%
echo OSPL_URI is %OSPL_URI%

rem setting variables 
SET SLEEP2=ping -n 5 127.0.0.1 
SET SLEEP5=ping 127.0.0.1 -n 5
@echo off
rem Standalone
echo %CD% > here
set EXPECTED_RESULT=..\..\..\..\expected_results
set WIN_BATCH="..\..\..\..\..\..\win_batch"
rem Corba
FIND /C "Corba" here > nul
if %ERRORLEVEL%==0 (
  set EXPECTED_RESULT=..\..\..\..\..\expected_results
  set WIN_BATCH="..\..\..\..\..\..\..\win_batch"
)
del /F /Q here

%VG_OSPL_START% ospl start
REM %VG_START_SLEEP%
echo "=== Launching Ownership "

echo === starting publisher "pub1" with ownership strength 5
start /B startPublisher.bat "pub1" 5 40 1	
echo === Waiting 2 seconds ...
%SLEEP2% >NUL
echo === starting publisher "pub2" with ownership strength 10
start /B  startPublisher.bat "pub2" 10 5 0 

call startSubscriber.bat subResult.txt
%SLEEP2% >NUL
rem Checking result of subscriber
FIND /C /I "pub1" subResult.txt  > nul
if NOT %ERRORLEVEL%==0 GOTO Err
FIND /C /I "pub2" subResult.txt  > nul
if NOT %ERRORLEVEL%==0 GOTO Err

echo OK
GOTO End
:Err
echo "*** ERROR : example Ownership failed ";
:End

rem Don't kill it too soon.
%SLEEP5% >NUL
%VG_OSPL_START% ospl stop
%SLEEP2% >NUL