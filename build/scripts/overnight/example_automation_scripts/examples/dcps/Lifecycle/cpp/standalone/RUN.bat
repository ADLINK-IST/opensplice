@echo OFF
SETLOCAL

echo "===== calling runLifecycle for sacpp ====="

set EXAMPLE_LANG=sacpp

call %FUNCTIONS% :runLifecycle

call %FUNCTIONS% :lifecycleCheckResults >> run.log