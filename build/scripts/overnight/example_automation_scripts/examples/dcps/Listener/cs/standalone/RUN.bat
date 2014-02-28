@echo OFF
SETLOCAL

echo "===== calling Listener for sacs ====="

set EXAMPLE_LANG=sacs

call %FUNCTIONS% :runListener

call %FUNCTIONS% :listenerCheckResults >> run.log