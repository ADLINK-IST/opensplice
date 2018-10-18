@echo OFF
SETLOCAL

set EXAMPLE_LANG=c
set NAME=Throughtput

call %FUNCTIONS% :runZeroThroughput

call %FUNCTIONS% :checkResultZero
