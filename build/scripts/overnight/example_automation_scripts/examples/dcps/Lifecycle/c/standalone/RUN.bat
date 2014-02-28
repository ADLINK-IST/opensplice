@echo OFF
SETLOCAL

echo "===== calling runLifecycle for sac ====="

set EXAMPLE_LANG=sac

call %FUNCTIONS% :runLifecycle

call %FUNCTIONS% :lifecycleCheckResults >> run.log