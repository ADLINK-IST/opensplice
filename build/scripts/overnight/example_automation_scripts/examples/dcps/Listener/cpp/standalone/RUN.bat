@echo OFF
SETLOCAL

echo "===== calling Listener for sacpp ====="

set EXAMPLE_LANG=sacpp

call %FUNCTIONS% :runListener

call %FUNCTIONS% :listenerCheckResults >> run.log