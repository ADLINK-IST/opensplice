@ECHO OFF

REM #####################
REM ErrorChecking section
REM #####################

set SLEEP5=@C:\WINDOWS\system32\ping.exe -n 5 localhost

SET FAIL=0
SET MESSAGE=FAILED
REM ignore the print out from ospl describing the location of the error log
ECHO Checking log file "< %LOGFILE% >" for errors 
FindStr -i "error" %LOGFILE% | findstr /v "ospl-error.log"
if %ERRORLEVEL% EQU 0 (
   SET FAIL=1
   SET MESSAGE=" - Errors occurred"
)
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
   SET MESSAGE=" - NoClassDefFound errors"
)
findstr /i /c:"assertion failed" %LOGFILE%
if %ERRORLEVEL% EQU 0 (
   SET FAIL=1
   SET MESSAGE=" - Assertions failures"
)
if EXIST "%OSPL_HOME%examples\%EXAMPLE%\ospl-error.log" (
   SET FAIL=1
   SET MESSAGE=" - ospl-error.log found"
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