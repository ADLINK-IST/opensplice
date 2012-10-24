@echo off
set pattern=%1
FIND /C %pattern% %2 > nul
if NOT %ERRORLEVEL%==0 echo *** Error  %pattern% not found in %2