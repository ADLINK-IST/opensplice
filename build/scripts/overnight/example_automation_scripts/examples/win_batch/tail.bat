@echo off
rem tail.bat -d <lines> <file>
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION
IF NOT "%1"=="-d" GOTO Err
IF "%2"=="" GOTO Err
IF "%3"=="" GOTO Err

:displayfile
SET skiplines=%2
SET sourcefile=%3

rem *** Get the current line count of file ***
FOR /F "usebackq tokens=3,3 delims= " %%l IN (`find /c /v "" %sourcefile%`) DO (call SET find_lc=%%l)
rem *** Calculate the lines to skip
SET /A skiplines=%find_lc%-!skiplines!
rem *** Display to screen line needed
more +%skiplines% %sourcefile%

GOTO end
:err
echo *** usage : tail.bat -d ^<lines^> ^<file^>
GOTO end

:end
