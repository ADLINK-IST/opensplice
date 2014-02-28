@echo OFF
SETLOCAL

echo "===== calling WaitSet for sac ====="

set EXAMPLE_LANG=sac

call %FUNCTIONS% :runWaitSet

call %FUNCTIONS% :waitsetCheckResults >> run.log



