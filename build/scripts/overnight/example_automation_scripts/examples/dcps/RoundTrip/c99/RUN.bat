@echo OFF
SETLOCAL

set EXAMPLE_LANG=c99
set NAME=RoundTrip
set SUB_PARAMS=
set PUB_PARAMS=

call %FUNCTIONS% :runZeroRoundTrip

call %FUNCTIONS% :checkResultZero
