@echo off
fc /W %1 %2 > nul
if NOT "%errorlevel%" == "0" (
   echo *** Error  %1 and %2 differs
   ) else (
   echo.
   echo Ok
)