@echo OFF
SETLOCAL

echo "===== calling WaitSet for sacpp ====="

set EXAMPLE_LANG=sacpp

call %FUNCTIONS% :runWaitSet

call %FUNCTIONS% :waitsetCheckResults >> run.log
