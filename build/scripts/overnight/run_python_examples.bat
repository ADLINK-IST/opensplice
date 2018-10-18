@ECHO OFF

SETLOCAL

pwd

call setenv.bat

REM RUN_LOG is a logfile containing the full test results

ECHO cd %OSPL_HOME% (OSPL_HOME)
cd "%OSPL_HOME%"

ECHO Set OSPL runtime environment
call release.bat
set PATH=c:\Python27;%PATH%
set HOSTNAME

REM run set to see what is actually set
set

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

ECHO cd %OSPLI_BASE% (OSPLI_BASE)
cd %OSPLI_BASE%
cd python

python runExample.py -a 

ENDLOCAL
