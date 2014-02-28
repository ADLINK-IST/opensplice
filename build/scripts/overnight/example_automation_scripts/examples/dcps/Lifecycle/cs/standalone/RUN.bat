
@echo OFF
SETLOCAL

echo "===== calling runLifecycle for sacs ====="

set EXAMPLE_LANG=sacs

call %FUNCTIONS% :runLifecycle

call %FUNCTIONS% :lifecycleCheckResults >> run.log
