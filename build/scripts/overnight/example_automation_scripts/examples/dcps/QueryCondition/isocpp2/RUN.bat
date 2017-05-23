@echo OFF
SETLOCAL

set EXAMPLE_LANG=isocpp
set NAME=QueryCondition
set SUB_PARAMS=GE
set PUB_PARAMS=

call %FUNCTIONS% :runZero

call %FUNCTIONS% :checkResultZero
