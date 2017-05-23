@echo OFF
SETLOCAL

set EXAMPLE_LANG=isocpp
set NAME=Throughtput

call %FUNCTIONS% :runZeroThroughputJava

call %FUNCTIONS% :checkResultZero
