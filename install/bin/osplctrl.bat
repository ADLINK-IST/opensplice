CALL "%~dp0\..\release.bat"
@"%OSPL_HOME%\bin\ospl.exe" %1
if %ERRORLEVEL% EQU 0 goto success

:failure
@echo off
set /p var=Please press Return to close the terminal.
goto end

:success
call :gettime t1
call :wait 300
call :gettime t2
set /A tt = (t2-t1)/100
goto :end

:wait
call :gettime wait0
:w2
call :gettime wait1
set /A waitt = wait1-wait0
if %waitt% lss %1 goto :w2
goto :end

:gettime
set hh=%time:~0,2%
set mm=%time:~3,2%
set ss=%time:~6,2%
set cc=%time:~-2%
set /A %1=hh*360000+mm*6000+ss*100+cc

:end
