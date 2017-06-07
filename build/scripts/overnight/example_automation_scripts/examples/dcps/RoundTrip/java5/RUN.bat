@echo OFF
SETLOCAL

set EXAMPLE_LANG=java5
set NAME=RoundTrip
set SUB_PARAMS=
set PUB_PARAMS=

call %FUNCTIONS% :runZeroRoundTripJava5

call %FUNCTIONS% :checkResultZero
