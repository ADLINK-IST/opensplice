@ECHO OFF

SET FAILURES=0
SET PASS=0
SET COUNT=0
SET SLEEP10=@ping localhost -c 10
SET SLEEP4=@ping localhost -c 4

ECHO Set required environment variables

call setenv.bat

REM RUN_LOG is a logfile containing the full test results

SET RUN_LOG="%LOGDIR%/examples/run/run_results.txt"

REM RUN_SUMMARY_LOG is a logfile containing an entry as to whether the example
REM passed or failed it is appended to the overview log at the end

SET RUN_SUMMARY_LOG="%LOGDIR%/examples/run/run_results_summary.txt"

REM SUMMARY_LOG Gives the number of examples run, the number that passed and
REM the number that failed together with the result of each individual examples

SET SUMMARY_LOG="%LOGDIR%/examples/run/examples.log"

ECHO Running examples and LOGDIR is %LOGDIR%

ECHO Set Microsoft Visual Studio Environment using VS supplied batch file

IF "%VS_ENV_SCRIPT%"=="" EXIT 1

IF NOT "%VS_ENV_SCRIPT%"=="" call "%VS_ENV_SCRIPT%"

cd "%OSPL_HOME%"

ECHO Set OSPL runtime environment
call release.bat
mkdir "%OSPL_HOME%\etc\tmp"
IF "%TMP%"=="" set TMP=%OSPL_HOME%\etc\tmp
IF "%TEMP%"=="" set TEMP=%OSPL_HOME%\etc\tmp

ECHO Change to the examples directory

cd "%OSPL_HOME%/examples"
sh ./set_xml.sh
call swap_URI.bat

SET LOGFILE=run.log

:SACPingPong
ECHO Run C examples
SET EXAMPLE=dcps/standalone/C/PingPong
SET NEXT=SACTutorial
CALL :RunExample

:SACTutorial
SET EXAMPLE=dcps/standalone/C/Tutorial
SET NEXT=SACPPPingPong
CALL :RunExample

:SACPPPingPong
ECHO Run C++ examples
SET EXAMPLE=dcps/standalone/C++/PingPong
SET NEXT=SACPPTutorial
CALL :RunExample

:SACPPTutorial
SET EXAMPLE=dcps/standalone/C++/Tutorial
SET NEXT=SACPPOnCPingPong
CALL :RunExample

:SACPPOnCPingPong
SET EXAMPLE=dcps/standalone/C++OnC/PingPong
SET NEXT=SAJPingPong
CALL :RunExample

:SAJPingPong
ECHO Run Java examples
SET EXAMPLE=dcps/standalone/Java/PingPong
SET NEXT=SAJTutorial
CALL :RunExample

:SAJTutorial
SET EXAMPLE=dcps/standalone/Java/Tutorial
SET NEXT=CJPingPong
CALL :RunExample

:CJPingPong
ECHO Run CORBA Java examples
SET EXAMPLE=dcps/CORBA/Java/JacORB/PingPong
SET NEXT=CJTutorial
CALL :RunExample

:CJTutorial
SET EXAMPLE=dcps/CORBA/Java/JacORB/Tutorial
SET NEXT=CCPPPingPong
CALL :RunExample

:CCPPPingPong
ECHO Run CORBA C++ examples
SET EXAMPLE=dcps/CORBA/C++/OpenFusion/PingPong
SET NEXT=CCPPTutorial
CALL :RunExample

:CCPPTutorial
SET EXAMPLE=dcps/CORBA/C++/OpenFusion/Tutorial
SET NEXT=DLRL
REM SET NEXT=SACPPHelloWorld
CALL :RunExample

:DLRL
ECHO Run dlrl examples
cd "%OSPL_HOME%/examples"
ECHO Check for DLRL directory
IF NOT EXIST "%OSPL_HOME%/examples/dlrl/" (GOTO NODLRL) ELSE (GOTO DLRLSAJTutorial)
REM Excluded because we dont have a BUILD.bat file
REM SET EXAMPLE=dlrl/standalone/C++/Tutorial
REM SET NEXT=DLRLSAJTutorial
REM CALL :RunExample

:DLRLSAJTutorial
SET EXAMPLE=dlrl/standalone/Java/Tutorial
SET NEXT=Done
CALL :RunExample

ECHO Run dbmsconnect example - not run at present as it fails 
REM SET EXAMPLE=services/dbmsconnect/SQL/C++/ODBC
REM CALL :RunExample

:Done
ECHO Finished running standard examples, start running new examples
ECHO DO NOT RUN THE REMAINING EXAMPLES 
REM call %OSPLI_BASE%/run_new_examples.bat %FAILURES% %PASS% %COUNT%
GOTO END

REM ###################
REM RunExample section
REM ##################

:RunExample

SET /A COUNT += 1
ECHO #####  %EXAMPLE%  #####
cd "%OSPL_HOME%/examples/%EXAMPLE%"
ECHO Starting %EXAMPLE% >> %LOGFILE%
%SLEEP4% >NUL
call RUN.bat >> %LOGFILE%
%SLEEP10% >NUL
CALL :ErrorChecking


REM #####################
REM ErrorChecking section
REM #####################

:ErrorChecking

SET ERROR=0
SET TIMEOUT=0
SET NOCLASSDEFFOUND=0
SET ASSERTFAILED=0
SET FAIL=0
REM ignore the print out from ospl describing the location of the error log
FindStr -i "error" %LOGFILE% | findstr /v "ospl-error.log"
if %ERRORLEVEL% EQU 0 SET ERROR=1
findstr /i /c:"timeout" %LOGFILE%
if %ERRORLEVEL% EQU 0 SET TIMEOUT=1 
findstr /i /c:"NoCLassDefFound" %LOGFILE%
if %ERRORLEVEL% EQU 0 SET NOCLASSDEFFOUND=1
findstr /i /c:"assertion failed" %LOGFILE%
if %ERRORLEVEL% EQU 0 SET ASSERTFAILED=1
if %ERROR% EQU 1 SET FAIL=1
if %TIMEOUT% EQU 1 SET FAIL=1
if %NOCLASSDEFFOUND% EQU 1 SET FAIL=1
if %ASSERTFAILED% EQU 1 SET FAIL=1
if EXIST %OSPL_HOME%examples\%EXAMPLE%\ospl-error.log SET FAIL=1
if EXIST %OSPL_HOME%examples\%EXAMPLE%\ospl-error.log echo ospl-error.log found for %EXAMPLE% >> %RUN_SUMMARY_LOG%
findstr /c:"WARNING" %OSPL_HOME%examples\%EXAMPLE%\ospl-info.log
if %ERRORLEVEL% EQU 0 SET FAIL=1
findstr /c:"WARNING" %OSPL_HOME%examples\%EXAMPLE%\ospl-info.log
if %ERRORLEVEL% EQU 0 echo WARNING found in ospl-info.log for %EXAMPLE% >> %RUN_SUMMARY_LOG%
REM Append result to %LOGFILE%
if %FAIL% EQU 1 (echo Run %EXAMPLE% FAILED >> %LOGFILE%) ELSE (echo Run %EXAMPLE% PASSED >> %LOGFILE%)
REM Append result to %RUN_SUMMARY_LOG%
if %FAIL% EQU 1 (echo Run %EXAMPLE% FAILED >> %RUN_SUMMARY_LOG%) ELSE (echo Run %EXAMPLE% PASSED >> %RUN_SUMMARY_LOG%)
if %FAIL% EQU 1 (SET /A FAILURES += 1) ELSE ( SET /A PASS += 1) 
REM The following line creates a string which is used to create a directory to hold
REM all the logs from the example run.
echo %EXAMPLE% | sed "s/standalone/SA/; s/CORBA/C/; s/Java/J/; s/C++/CPP/; s/JacORB//; s/OpenFusion//; s/\///g;" > tmp.txt
set /p EXAMPLELOGS= < tmp.txt
mkdir %LOGDIR%\examples\run\%EXAMPLELOGS%
cp *.log %LOGDIR%\examples\run\%EXAMPLELOGS%
ECHO %EXAMPLE% run complete
ECHO /////
ECHO /////
%SLEEP10% >NUL
ECHO Next example is %NEXT%
GOTO %NEXT%

:NODLRL
ECHO No DLRL examples to run
GOTO END

REM ##################
REM END
REM #################

:END
%SLEEP10% >NUL
echo .. >> %SUMMARY_LOG%
echo ################## Summary of build ############ >> %SUMMARY_LOG%
echo Examples Run   : %COUNT% >> %SUMMARY_LOG%
echo Runs Passed    : %PASS% >> %SUMMARY_LOG%
echo Runs Failed    : %FAILURES% >> %SUMMARY_LOG%
echo ################################################ >> %SUMMARY_LOG%
echo .. >> %SUMMARY_LOG%
echo RESULTS >> %SUMMARY_LOG%
%SLEEP10% >NUL
REM We need to go the the directory where the logs are in order to append
REM the individual example results to the examples.log that is uploaded to 
REM the scoreboard.
cd %LOGDIR%\examples\run
type run_results_summary.txt >> examples.log
REM try the above as this following line doesnt appear to work
REM type %RUN_SUMMARY_LOG% >> %SUMMARY_LOG%
%SLEEP10% >NUL
REM Delete the run_results_summary.txt file as we have appended the
REM contents to the examples.log which is backed up to the scoreboard
del run_results_summary.txt
%SLEEP10% > NUL
REM For some reason an ospl_info.log gets written to the log directory as
REM a result of the above, we don't need it and we don't want it added to
REM the scoreboard, so just delete it here
if exist ospl*.log del ospl*.log
cd "%OSPL_HOME%"
cd ..\..\
REM echo Uninstalling 
REM if exist uninstall-x86.win32-HDE.exe uninstall-x86.win32-HDE.exe --mode unattended
%SLEEP10% >NUL
if NOT %FAILURES% EQU 0 EXIT 1
EXIT 0
