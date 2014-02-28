echo OFF
SETLOCAL

echo "===== calling Ownership for sacpp ====="

set EXAMPLE_LANG=sacpp

call %FUNCTIONS% :runOwnership

call %FUNCTIONS% :ownershipCheckResult >> run.log
