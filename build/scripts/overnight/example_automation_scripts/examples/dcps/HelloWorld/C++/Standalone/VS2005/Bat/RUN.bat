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
set EXPECTED_RESULT=..\..\..\..\expected_results
set WIN_BATCH="..\..\..\..\..\..\win_batch"
rem Corba
echo %CD% > here
FIND /C "Corba" here > nul
if %ERRORLEVEL%==0 (
  set EXPECTED_RESULT=..\..\..\..\..\expected_results
  set WIN_BATCH="..\..\..\..\..\..\..\win_batch"
)
del /F /Q here

%VG_OSPL_START% ospl start
REM %VG_START_SLEEP%
echo "=== Launching HelloWorld "
call startPublisher.bat pubResult.txt
%SLEEP2% >NUL
call startSubscriber.bat subResult.txt
echo "=== Checking HelloWorld Subscriber results"
echo === EXPECTED_RESULT ===
type %EXPECTED_RESULT%\subResult.txt
echo.
call %WIN_BATCH%\tail.bat -d 3 subResult.txt > tail_subResult.txt
call %WIN_BATCH%\diff.bat tail_subResult.txt %EXPECTED_RESULT%\subResult.txt
rem Don't kill it too soon.
%SLEEP5% >NUL
%VG_OSPL_START% ospl stop
%SLEEP2% >NUL
