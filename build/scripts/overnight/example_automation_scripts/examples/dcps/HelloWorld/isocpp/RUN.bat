@echo OFF
SETLOCAL

set EXAMPLE_LANG=isocpp
set NAME=HelloWorld
set SUB_PARAMS=
set PUB_PARAMS=

call %FUNCTIONS% :runZero

call %FUNCTIONS% :checkResultZero
