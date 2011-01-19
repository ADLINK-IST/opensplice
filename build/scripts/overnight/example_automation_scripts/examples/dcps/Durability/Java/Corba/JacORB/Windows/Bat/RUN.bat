@echo OFF
del /F /Q C:\tmp\pstore
del /F /Q *.log
del /F /Q *.txt

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
del /F here

%VG_OSPL_START% ospl start
REM %VG_START_SLEEP%
echo "=== Launching Durability "
call start.bat
rem ================== Scenario 3.1 ========================
echo "=== Checking Durability Subscriber results Scenario 3.1"
call %WIN_BATCH%\tail.bat -d 10 subResult_3.1.txt > tail_subResult.txt
call %WIN_BATCH%\tail.bat -d 10 %EXPECTED_RESULT%\subResult_3.1.txt > tail_expected_result
fc /W tail_subResult.txt tail_expected_result > nul
if NOT "%errorlevel%" == "0" GOTO Err
rem ================== Scenario 3.2 ========================
echo "=== Checking Durability second Subscriber results Scenario 3.2"
rem Checking only result of second subscriber
echo "=== Checking Durability Subscriber results Scenario 3.2"
call %WIN_BATCH%\tail.bat -d 10 subResult_3.2.2.txt > tail_subResult.txt
call %WIN_BATCH%\tail.bat -d 10 %EXPECTED_RESULT%\subResult_3.2.2.txt > tail_expected_result
fc /W tail_subResult.txt tail_expected_result > nul
if NOT "%errorlevel%" == "0" GOTO Err
rem ================== Scenario 3.2 ========================
echo "=== Checking Durability second Subscriber results Scenario 3.3"
echo "    (not empty after restarting OpenSplice)"
rem checking only that result of second subscriber is not empty
call %WIN_BATCH%\tail.bat type subResult_3.3.2.txt > tail_subResult.txt
dir subResult_3.3.2.txt |find "subResult_3.3.2.txt" > resnul.txt
for /f "tokens=3" %%i in (resnul.txt) do set SIZE=%%i
if "%SIZE%" == "0" GOTO Err

echo OK
GOTO End
:Err
echo "*** ERROR : example Durability failed ";
:End

rem Don't kill it too soon.
%SLEEP5% >NUL
%VG_OSPL_START% ospl stop
%SLEEP2% >NUL