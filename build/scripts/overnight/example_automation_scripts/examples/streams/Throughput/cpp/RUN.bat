@echo OFF
SETLOCAL

set EXAMPLE_LANG=cpp
set NAME=StreamsThroughtput

call %FUNCTIONS% :runZeroStreamsThroughput

call %FUNCTIONS% :checkResultZero
