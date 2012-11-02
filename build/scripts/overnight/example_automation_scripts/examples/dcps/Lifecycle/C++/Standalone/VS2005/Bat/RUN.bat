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
SET SLEEP4=ping 127.0.0.1 -n 4
SET SLEEP5=ping 127.0.0.1 -n 5

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

rem =======================================================
rem ==                    step_1                         ==
rem =======================================================

%VG_OSPL_START% ospl start
%SLEEP4% >NUL

echo "=== Launching Lifecycle "
echo "=== (step 1)" 
start /B startPublisher.bat false dispose > pubResult_1.txt
call startSubscriber.bat subResult_1.txt
%VG_OSPL_START% ospl stop
sleep 2
echo "=== (step_1) Checking Lifecycle Subscriber results "
FIND /C /I "sample_state:NOT_READ_SAMPLE_STATE-view_state:NEW_VIEW_STATE-instance_state:ALIVE_INSTANCE_STATE" subResult_1.txt  > nul
if NOT %ERRORLEVEL%==0 GOTO Err_1
FIND /C /I "sample_state:READ_SAMPLE_STATE-view_state:NOT_NEW_VIEW_STATE-instance_state:ALIVE_INSTANCE_STATE" subResult_1.txt  > nul
if NOT %ERRORLEVEL%==0 GOTO Err_1
FIND /C /I "sample_state:READ_SAMPLE_STATE-view_state:NOT_NEW_VIEW_STATE-instance_state:NOT_ALIVE_DISPOSED_INSTANCE_STATE" subResult_1.txt  > nul
if NOT %ERRORLEVEL%==0 GOTO Err_1
echo step_1 OK
GOTO End_1
:Err_1
echo "*** (step_1) ERROR : example Lifecycle failed ";
type subResult_1.txt
:End_1

rem =======================================================
rem ==                    step_2                         ==
rem =======================================================
%VG_OSPL_START% ospl start
%VG_START_SLEEP%

echo "=== (step 2)" 
start /B startPublisher.bat false unregister > pubResult_2.txt
call startSubscriber.bat subResult_2.txt
%VG_OSPL_START% ospl stop
sleep 2
echo "=== (step_2) Checking Lifecycle Subscriber results "
FIND /C /I "sample_state:NOT_READ_SAMPLE_STATE-view_state:NEW_VIEW_STATE-instance_state:ALIVE_INSTANCE_STATE" subResult_2.txt  > nul
if NOT %ERRORLEVEL%==0 GOTO Err_2
FIND /C /I "sample_state:READ_SAMPLE_STATE-view_state:NOT_NEW_VIEW_STATE-instance_state:ALIVE_INSTANCE_STATE" subResult_2.txt  > nul
if NOT %ERRORLEVEL%==0 GOTO Err_2
FIND /C /I "sample_state:READ_SAMPLE_STATE-view_state:NOT_NEW_VIEW_STATE-instance_state:NOT_ALIVE_NO_WRITERS_INSTANCE_STATE" subResult_2.txt  > nul
if NOT %ERRORLEVEL%==0 GOTO Err_2
echo step_2 OK
GOTO End_2
:Err_2
echo "*** (step_2) ERROR : example Lifecycle failed ";
type subResult_2.txt
:End_2

rem =======================================================
rem ==                    step_3                         ==
rem =======================================================
%VG_OSPL_START% ospl start
%VG_START_SLEEP%

echo "=== (step 3)" 
start /B startPublisher.bat false unregister > pubResult_3.txt
call startSubscriber.bat subResult_3.txt

echo "=== (step_3) Checking Lifecycle Subscriber results "
FIND /C /I "sample_state:NOT_READ_SAMPLE_STATE-view_state:NEW_VIEW_STATE-instance_state:ALIVE_INSTANCE_STATE" subResult_3.txt  > nul
if NOT %ERRORLEVEL%==0 GOTO Err_3
FIND /C /I "sample_state:READ_SAMPLE_STATE-view_state:NOT_NEW_VIEW_STATE-instance_state:ALIVE_INSTANCE_STATE" subResult_3.txt  > nul
if NOT %ERRORLEVEL%==0 GOTO Err_3
FIND /C /I "sample_state:READ_SAMPLE_STATE-view_state:NOT_NEW_VIEW_STATE-instance_state:NOT_ALIVE_NO_WRITERS_INSTANCE_STATE" subResult_3.txt  > nul
if NOT %ERRORLEVEL%==0 GOTO Err_3
echo step_3 OK
GOTO End_3
:Err_3
echo "*** (step_3) ERROR : example Lifecycle failed ";
type subResult_3.txt
:End_3

rem Don't kill it too soon.
%SLEEP5% >NUL
%VG_OSPL_START% ospl stop
%SLEEP2% >NUL

