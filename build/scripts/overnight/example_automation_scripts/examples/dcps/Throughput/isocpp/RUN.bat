@echo OFF
SETLOCAL

set EXAMPLE_LANG=isocpp
set NAME=Throughtput

call %FUNCTIONS% :runZeroThroughput

call %FUNCTIONS% :checkResultZero
