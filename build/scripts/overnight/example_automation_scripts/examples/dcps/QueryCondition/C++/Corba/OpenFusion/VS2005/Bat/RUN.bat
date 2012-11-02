@echo OFF
if "%OSPL_HOME%" == "" (
    echo "*** Please source release.com to set OpenSplice environment variables";
    echo "!!! ERROR OpenSplice environment variables NOT SET" >> SubResult.txt
    exit;
)
echo OSPL_HOME is %OSPL_HOME%
echo OSPL_URI is %OSPL_URI%

rem setting variables 
SET SLEEP2=ping 127.0.0.1 -n 2
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

%VG_OSPL_START% ospl start
REM %VG_START_SLEEP%


echo "=== Launching QueryCondition "

start /B startPublisher.bat
%SLEEP2% >NUL
call startSubscriber.bat subResult.txt

echo "=== Checking QueryCondition Subscriber results"
FIND /C /I "MSFT" subResult.txt  > nul
if NOT %ERRORLEVEL%==0 GOTO Err
FIND /C /I "GE" subResult.txt  > nul
if NOT %ERRORLEVEL%==1 GOTO Err

echo OK
GOTO End
:Err
echo "*** ERROR : example QueryCondition failed ";
:End

rem Don't kill it too soon.
%VG_OSPL_START% ospl stop
%SLEEP2% >NUL

