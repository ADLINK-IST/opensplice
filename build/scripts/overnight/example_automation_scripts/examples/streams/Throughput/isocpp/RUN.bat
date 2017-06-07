@echo OFF
SETLOCAL

set EXAMPLE_LANG=isocpp
set NAME=StreamsThroughtput

call %FUNCTIONS% :runZeroStreamsThroughput

call %FUNCTIONS% :checkResultZero
