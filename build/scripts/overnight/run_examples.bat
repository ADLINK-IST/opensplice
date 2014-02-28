@ECHO OFF

SET FAILURES=0
SET PASS=0
SET SUM=0

set SLEEP2=@C:\WINDOWS\system32\ping.exe -n 2 localhost
set SLEEP4=@C:\WINDOWS\system32\ping.exe -n 4 localhost
set SLEEP5=@C:\WINDOWS\system32\ping.exe -n 5 localhost
set SLEEP10=@C:\WINDOWS\system32\ping.exe -n 10 localhost
set SLEEP30=@C:\WINDOWS\system32\ping.exe -n 30 localhost

ECHO Set required environment variables

call setenv.bat

set CHECKERRORS="%OSPLI_BASE%/checkErrors.bat"
set FUNCTIONS="%OSPLI_BASE%/example_automation_scripts/examples/Functions.bat"

REM RUN_LOG is a logfile containing the full test results

SET RUN_LOG="%LOGDIR%/examples/run_%EXRUNTYPE%/run_results.txt"

REM RUN_SUMMARY_LOG is a logfile containing an entry as to whether the example
REM passed or failed it is appended to the overview log at the end
SET RUN_SUMMARY_LOG="%LOGDIR%/examples/run_%EXRUNTYPE%/run_results_summary.txt"

SET TOTALS_LOG="%LOGDIR%/examples/run_%EXRUNTYPE%/totals.log"

REM SUMMARY_LOG Gives the number of examples run, the number that passed and
REM the number that failed together with the result of each individual examples

SET SUMMARY_LOG="%LOGDIR%/examples/run_%EXRUNTYPE%/examples.log"

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

REM some processes appear to get left running this
REM may be due to pop ups so try setting OSPL_DEBUG_LOGPATH
REM to see it that works see OSPL-555
set OSPL_DEBUG_LOGPATH=%TEMP%

ECHO Change to the examples directory

cd "%OSPL_HOME%/examples"
sh ./set_xml.sh
call swap_URI.bat

SET LOGFILE=run.log

FOR %%e in (%EXAMPLES%) DO (
   SET /A SUM += 1
   SET EXAMPLE=%%e
   ECHO #####  %%e  #####
   echo Changing directory to %OSPL_HOME%/examples/%%e
   cd "%OSPL_HOME%/examples/%%e" 
   ECHO Starting %%e >> %LOGFILE%
   %SLEEP4% >NUL
   call RUN.bat
   call %CHECKERRORS%
)

REM ##################
REM END
REM #################

:END
echo Changing directory to %OSPL_HOME%\examples
cd "%OSPL_HOME%\examples"
sh ./clean_xml.sh
%SLEEP10% >NUL
echo Changing directory to %LOGDIR%\examples\run_%EXRUNTYPE%
cd %LOGDIR%\examples\run_%EXRUNTYPE%

echo Examples run = %SUM% > %TOTALS_LOG%
echo Examples passed = %PASS% >> %TOTALS_LOG%
echo Examples failed = %FAILURES% >> %TOTALS_LOG%

if NOT %FAILURES% EQU 0 EXIT 1
EXIT 0
