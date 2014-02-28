@ECHO OFF

REM #####################
REM ErrorChecking section
REM #####################

set SLEEP5=@C:\WINDOWS\system32\ping.exe -n 5 localhost

SET FAIL=0
SET MESSAGE=
SET /a count=0
SET /a hbcount=0
REM ignore the print out from ospl describing the location of the error log
ECHO Checking log file "< %LOGFILE% >" for errors 
FindStr -i "error" %LOGFILE% | findstr /v "ospl-error.log"
if %ERRORLEVEL% EQU 0 (
   SET FAIL=1
   SET MESSAGE="FAILED - Errors occurred"
)
findstr /i /c:"failed" %LOGFILE%
if %ERRORLEVEL% EQU 0 (
   SET FAIL=1 
   SET MESSAGE="FAILED - Failures occurred"
)
REM This fancy stuff is not working.  I did have something working locally but it goes to po
REM once it's applied in the overnights.  Comment out so we get examples back and have a
REM look at it later.  Put the old stuff back in and check manually
REM ECHO Check for timeouts....
REM for /f %%c in ('C:\WINDOWS\system32\find.exe /i /c "timeout" ^< %LOGFILE%') do set count=%%c
REM if %count% neq 0 (
REM   SET MESSAGE=%count% - TIMEOUTS occurred
REM   SET OK=- 5 or less so thats OK
REM )
REM ECHO Check count > 5
REM if %count% gtr 5 (
REM   SET FAIL=1 
REM   SET MESSAGE=FAILED - %MESSAGE%
REM ) else
REM   SET MESSAGE=%MESSAGE% %OK%
REM )  
REM ECHO Check for WARNINGs....
REM set count=0
REM if EXIST "%OSPL_HOME%examples\%EXAMPLE%\ospl-info.log" (
REM   for /f %%c in ('C:\WINDOWS\system32\find.exe /c "WARNING" ^< %OSPL_HOME%examples\%EXAMPLE%\ospl-info.log') do set count=%%c
REM   for /f %%c in ('C:\WINDOWS\system32\find.exe /c "Missed heartbeat" ^< %OSPL_HOME%examples\%EXAMPLE%\ospl-info.log') do set hbcount=%%c
REM )
REM ECHO Check number of missing heartbeats......
REM if %count% neq 0 (
REM   SET MESSAGE=%count% - WARNINGS found in ospl-info.log
REM   SET OK=- %hbcount% Missed heartbeat WARNINGS - OK
REM )
REM ECHO Check if all warnings are missing heartbeats .....
REM if %count% equ %hbcount (
REM   SET MESSAGE=%MESSAGE% %OK%
REM ) else (
REM   SET FAIL=1 
REM   SET MESSAGE=FAILED - %MESSAGE% - %hbcount% Missed heartbeat WARNINGS
REM )
findstr /i /c:"timeout" %LOGFILE%
if %ERRORLEVEL% EQU 0 (
   SET FAIL=1 
   SET MESSAGE=" - TIMEOUTS occurred"
)
if EXIST "%OSPL_HOME%examples\%EXAMPLE%\ospl-info.log" (
   findstr /c:"WARNING" "%OSPL_HOME%examples\%EXAMPLE%\ospl-info.log"
)
if %ERRORLEVEL% EQU 0 (
   SET FAIL=1
   SET MESSAGE=" - Warnings found in ospl-info.log"
)
findstr /i /c:"NoCLassDefFound" %LOGFILE%
if %ERRORLEVEL% EQU 0 (
   SET FAIL=1
   SET MESSAGE="FAILED - NoClassDefFound errors"
)
findstr /i /c:"assertion failed" %LOGFILE%
if %ERRORLEVEL% EQU 0 (
   SET FAIL=1
   SET MESSAGE="FAILED - Assertions failures"
)
if EXIST "%OSPL_HOME%examples\%EXAMPLE%\ospl-error.log" (
   SET FAIL=1
   SET MESSAGE="FAILED - ospl-error.log found"
)
%SLEEP5% >NUL
ECHO Adding result to log file
REM Append result to %LOGFILE%
if %FAIL% EQU 1 (echo %EXAMPLE% FAILED %MESSAGE% >> %LOGFILE%) ELSE (echo %EXAMPLE% PASSED >> %LOGFILE%)
REM Append result to %RUN_SUMMARY_LOG%
ECHO Adding result to summary log file
REM The following line creates a string which is used to create a directory to hold
REM all the logs from the example run.
echo %EXAMPLE% | sed "s/standalone/SA/; s/corba/C/; s/java/J/; s/cpp/CPP/; s/OpenFusion//; s/\///g;" > tmp.txt
set /p EXAMPLELOGS= < tmp.txt
if %FAIL% EQU 1 (echo %EXAMPLELOGS% FAILED %MESSAGE% >> %RUN_SUMMARY_LOG%) ELSE (echo %EXAMPLELOGS% PASSED >> %RUN_SUMMARY_LOG%)
if %FAIL% EQU 1 (SET /A FAILURES += 1) ELSE ( SET /A PASS += 1) 
mkdir %LOGDIR%\examples\run_%EXRUNTYPE%\%EXAMPLELOGS%
ECHO Copying logs and result files to example results directory 
cp *.log %LOGDIR%\examples\run_%EXRUNTYPE%\%EXAMPLELOGS%
cp *Result*.txt %LOGDIR%\examples\run_%EXRUNTYPE%\%EXAMPLELOGS% 
ECHO %EXAMPLE% run complete
ECHO /////
ECHO /////
%SLEEP5% >NUL