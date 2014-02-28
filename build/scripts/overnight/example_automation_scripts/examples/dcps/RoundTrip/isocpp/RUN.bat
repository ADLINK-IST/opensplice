@echo OFF
SETLOCAL

set EXAMPLE_LANG=isocpp
set NAME=RoundTrip
set SUB_PARAMS=
set PUB_PARAMS=

call %FUNCTIONS% :runZeroRoundTrip

call %FUNCTIONS% :checkResultZero
