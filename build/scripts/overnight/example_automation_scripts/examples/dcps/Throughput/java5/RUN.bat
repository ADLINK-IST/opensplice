@echo OFF
SETLOCAL

set EXAMPLE_LANG=java5
set NAME=Throughtput

call %FUNCTIONS% :runZeroThroughputJava5

call %FUNCTIONS% :checkResultZero
