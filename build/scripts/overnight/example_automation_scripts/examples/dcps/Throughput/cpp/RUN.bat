@echo OFF
SETLOCAL

set EXAMPLE_LANG=cpp
set NAME=Throughtput

call %FUNCTIONS% :runZeroThroughput

call %FUNCTIONS% :checkResultZero
