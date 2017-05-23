@echo OFF
SETLOCAL

echo "===== calling Ownership for sac ====="

set EXAMPLE_LANG=sac

call %FUNCTIONS% :runOwnership

call %FUNCTIONS% :ownershipCheckResults >> run.log
