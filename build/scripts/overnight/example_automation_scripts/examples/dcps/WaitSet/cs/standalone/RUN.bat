@ECHO OFF

SETLOCAL

echo "===== calling WaitSet for sacs  ====="

set EXAMPLE_LANG=sacs

call %FUNCTIONS% :runWaitSet

call %FUNCTIONS% :waitsetCheckResults >> run.log
