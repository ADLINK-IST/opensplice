@ECHO OFF


REM This is work in progress.  All the examples cannot be in 1 file
REM as this causes problems.  We need a better solution.  This file
REM is temporary till that solution is found.  Also the batch files
REM with the new examples don't work on the overnight test runs so 
REM we need to fix them first.  I suggest 1 by 1 and not altogether
REM e.g. get the HelloWorld example working first then introduce the
REM others incrementally

SET FAILURES=%1
SET PASS=%2
SET COUNT=%3
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

REM IF "%VS_ENV_SCRIPT%"=="" EXIT 1

REM IF NOT "%VS_ENV_SCRIPT%"=="" call "%VS_ENV_SCRIPT%"

REM cd "%OSPL_HOME%"

REM ECHO Set OSPL runtime environment
REM call release.bat

REM mkdir "%OSPL_HOME%\etc\tmp"
REM IF "%TMP%"=="" set TMP=%OSPL_HOME%\etc\tmp
REM IF "%TEMP%"=="" set TEMP=%OSPL_HOME%\etc\tmp

ECHO Change to the examples directory

cd "%OSPL_HOME%/examples"
REM sh ./set_xml.sh
REM call swap_URI.bat

SET LOGFILE=run.log

REM :SACPPHelloWorld
REM SET EXAMPLE=dcps/HelloWorld/C++/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SACHelloWorld
REM CALL :RunNewExample

REM :SACHelloWorld
REM SET EXAMPLE=dcps/HelloWorld/C/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SAJHelloWorld
REM CALL :RunNewExample

REM :SAJHelloWorld
REM SET EXAMPLE=dcps/HelloWorld/Java/Standalone/Windows/Bat
REM SET NEXTEXAMPLE=CCPPHelloWorld
REM CALL :RunNewExample

REM :CCPPHelloWorld
REM SET EXAMPLE=dcps/HelloWorld/C++/OpenFusion/VS2005/Bat
REM SET NEXTEXAMPLE=CJHelloWorld
REM CALL :RunNewExample

REM :CJHelloWorld
REM SET EXAMPLE=dcps/HelloWorld/Java/Corba/JacORB/Windows/Bat
REM SET NEXTEXAMPLE=SACSHelloWorld
REM CALL :RunNewExample

REM :SACSHelloWorld
REM SET EXAMPLE=dcps/HelloWorld/CS/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=DoneNew
REM CALL :RunNewExample

REM  :SACPPContentFilteredTopic
REM SET EXAMPLE=dcps/ContentFilteredTopic/C++/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SACContentFilteredTopic
REM CALL :RunNewExample

REM :SACContentFilteredTopic
REM SET EXAMPLE=dcps/ContentFilteredTopic/C/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SAJContentFilteredTopic
REM CALL :RunNewExample

REM :SAJContentFilteredTopic
REM SET EXAMPLE=dcps/ContentFilteredTopic/Java/Standalone/Windows/Bat
REM SET NEXTEXAMPLE=CCPPContentFilteredTopic
REM CALL :RunNewExample

REM :CCPPContentFilteredTopic
REM SET EXAMPLE=dcps/ContentFilteredTopic/C++/OpenFusion/VS2005/Bat
REM SET NEXTEXAMPLE=CJContentFilteredTopic
REM CALL :RunNewExample

REM :CJContentFilteredTopic
REM SET EXAMPLE=dcps/ContentFilteredTopic/Java/Corba/JacORB/Windows/Bat
REM SET NEXTEXAMPLE=SACSContentFilteredTopic
REM CALL :RunNewExample

REM :SACSContentFilteredTopic
REM SET EXAMPLE=dcps/ContentFilteredTopic/CS/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SACPPListener
REM CALL :RunNewExample

REM :SACPPListener
REM SET EXAMPLE=dcps/Listener/C++/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SACListener
REM CALL :RunNewExample

REM :SACListener
REM SET EXAMPLE=dcps/Listener/C/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SAJListener
REM CALL :RunNewExample

REM :SAJListener
REM SET EXAMPLE=dcps/Listener/Java/Standalone/Windows/Bat
REM SET NEXTEXAMPLE=CCPPListener
REM CALL :RunNewExample

REM :CCPPListener
REM SET EXAMPLE=dcps/Listener/C++/OpenFusion/VS2005/Bat
REM SET NEXTEXAMPLE=CJListener
REM CALL :RunNewExample

REM :CJListener
REM SET EXAMPLE=dcps/Listener/Java/Corba/JacORB/Windows/Bat
REM SET NEXTEXAMPLE=SACSListener
REM CALL :RunNewExample

REM :SACSListener
REM SET EXAMPLE=dcps/Listener/CS/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SACPPOwnership
REM CALL :RunNewExample

REM :SACPPOwnership
REM SET EXAMPLE=dcps/Ownership/C++/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SACOwnership
REM CALL :RunNewExample

REM :SACOwnership
REM SET EXAMPLE=dcps/Ownership/C/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SAJOwnership
REM CALL :RunNewExample

REM :SAJOwnership
REM SET EXAMPLE=dcps/Ownership/Java/Standalone/Windows/Bat
REM SET NEXTEXAMPLE=CCPPOwnership
REM CALL :RunNewExample

REM :CCPPOwnership
REM SET EXAMPLE=dcps/Ownership/C++/OpenFusion/VS2005/Bat
REM SET NEXTEXAMPLE=CJOwnership
REM CALL :RunNewExample

REM :CJOwnership
REM SET EXAMPLE=dcps/Ownership/Java/Corba/JacORB/Windows/Bat
REM SET NEXTEXAMPLE=SACSOwnership
REM CALL :RunNewExample

REM :SACSOwnership
REM SET EXAMPLE=dcps/Ownership/CS/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SACPPQueryCondition
REM CALL :RunNewExample

REM :SACPPQueryCondition
REM SET EXAMPLE=dcps/QueryCondition/C++/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SACQueryCondition
REM CALL :RunNewExample

REM :SACQueryCondition
REM SET EXAMPLE=dcps/QueryCondition/C/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SAJQueryCondition
REM CALL :RunNewExample

REM :SAJQueryCondition
REM SET EXAMPLE=dcps/QueryCondition/Java/Standalone/Windows/Bat
REM SET NEXTEXAMPLE=CCPPQueryCondition
REM CALL :RunNewExample

REM :CCPPQueryCondition
REM SET EXAMPLE=dcps/QueryCondition/C++/OpenFusion/VS2005/Bat
REM SET NEXTEXAMPLE=CJQueryCondition
REM CALL :RunNewExample

REM :CJQueryCondition
REM SET EXAMPLE=dcps/QueryCondition/Java/Corba/JacORB/Windows/Bat
REM SET NEXTEXAMPLE=SACSQueryCondition
REM CALL :RunNewExample

REM :SACSQueryCondition
REM SET EXAMPLE=dcps/QueryCondition/CS/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SACPPWaitSet
REM CALL :RunNewExample

REM :SACPPWaitSet
REM SET EXAMPLE=dcps/WaitSet/C++/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SACWaitSet
REM CALL :RunNewExample

REM :SACWaitSet
REM SET EXAMPLE=dcps/WaitSet/C/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SAJWaitSet
REM CALL :RunNewExample

REM :SAJWaitSet
REM SET EXAMPLE=dcps/WaitSet/Java/Standalone/Windows/Bat
REM SET NEXTEXAMPLE=CCPPWaitSet
REM CALL :RunNewExample

REM :CCPPWaitSet
REM SET EXAMPLE=dcps/WaitSet/C++/OpenFusion/VS2005/Bat
REM SET NEXTEXAMPLE=CJWaitSet
REM CALL :RunNewExample

REM :CJWaitSet
REM SET EXAMPLE=dcps/WaitSet/Java/Corba/JacORB/Windows/Bat
REM SET NEXTEXAMPLE=SACSWaitSet
REM CALL :RunNewExample

REM :SACSWaitSet
REM SET EXAMPLE=dcps/WaitSet/CS/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SACPPBuiltInTopics
REM CALL :RunNewExample

REM :SACPPBuiltInTopics
REM SET EXAMPLE=dcps/BuiltInTopics/C++/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SACBuiltInTopics
REM CALL :RunNewExample
 
REM :SACBuiltInTopics
REM SET EXAMPLE=dcps/BuiltInTopics/C/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SAJBuiltInTopics
REM CALL :RunNewExample
 
REM :SAJBuiltInTopics
REM SET EXAMPLE=dcps/BuiltInTopics/Java/Standalone/Windows/Bat
REM SET NEXTEXAMPLE=CCPPBuiltInTopics
REM CALL :RunNewExample
 
REM :CCPPBuiltInTopics
REM SET EXAMPLE=dcps/BuiltInTopics/C++/OpenFusion/VS2005/Bat
REM SET NEXTEXAMPLE=CJBuiltInTopics
REM CALL :RunNewExample
 
REM :CJBuiltInTopics
REM SET EXAMPLE=dcps/BuiltInTopics/Java/Corba/JacORB/Windows/Bat
REM SET NEXTEXAMPLE=SACSBuiltInTopics
REM CALL :RunNewExample

REM :SACSBuiltInTopics
REM SET EXAMPLE=dcps/BuiltInTopics/CS/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=DLRL
REM CALL :RunNewExample

REM DO NOT run Durability it hangs when run with overnights
REM :SACPPDurability
REM SET EXAMPLE=dcps/Durability/C++/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SACDurability
REM CALL :RunNewExample

REM :SACDurability
REM SET EXAMPLE=dcps/Durability/C/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SAJDurability
REM CALL :RunNewExample

REM :SAJDurability
REM SET EXAMPLE=dcps/Durability/Java/Standalone/Windows/Bat
REM SET NEXTEXAMPLE=CCPPDurability
REM CALL :RunNewExample

REM :CCPPDurability
REM SET EXAMPLE=dcps/Durability/C++/OpenFusion/VS2005/Bat
REM SET NEXTEXAMPLE=CCDurability
REM CALL :RunNewExample

REM :CJDurability
REM SET EXAMPLE=dcps/Durability/Java/Corba/JacORB/Windows/Bat
REM SET NEXTEXAMPLE=SACSDurability
REM CALL :RunNewExample

REM :SACSDurability
REM SET EXAMPLE=dcps/Durability/C/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SACPPListener
REM CALL :RunNewExample

REM DON NOT run - Lifecycle hangs when run with overnights
REM :SACPPLifecycle
REM SET EXAMPLE=dcps/Lifecycle/C++/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SACLifecycle
REM CALL :RunNewExample

REM :SACLifecycle
REM SET EXAMPLE=dcps/Lifecycle/C/Standalone/VS2005/Bat
REM SET NEXTEXAMPLE=SAJLifecycle
REM CALL :RunNewExample

REM :SAJLifecycle
REM SET EXAMPLE=dcps/Lifecycle/Java/Standalone/Windows/Bat
REM SET NEXTEXAMPLE=CCPPLifecycle
REM CALL :RunNewExample

REM :CCPPLifecycle
REM SET EXAMPLE=dcps/Lifecycle/C++/OpenFusion/VS2005/Bat
REM SET NEXTEXAMPLE=CCLifecycle
REM CALL :RunNewExample

REM :CJLifecycle
REM SET EXAMPLE=dcps/Lifecycle/Java/Corba/JacORB/Windows/Bat
REM SET NEXTEXAMPLE=done
REM CALL :RunNewExample

:DoneNew
ECHO Ran limited set of new examples till batch files fixed
GOTO END

REM ###################
REM RunNewExample section
REM ##################

:RunNewExample

SET /A COUNT += 1
ECHO #####  %EXAMPLE%  #####
cd "%OSPL_HOME%/examples/%EXAMPLE%"
ECHO Starting %EXAMPLE% >> %LOGFILE%
%SLEEP4% >NUL
call RUN.bat >> %LOGFILE%
%SLEEP10% >NUL
CALL :ErrorCheckingResult


REM #####################
REM ErrorCheckingResult section
REM #####################

:ErrorCheckingResult

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
ECHO Next example is %NEXTEXAMPLE%
GOTO %NEXTEXAMPLE%

:END
ECHO End of new examples 
