
@ECHO off
SETLOCAL

echo "===== calling Ownership for sacs ====="

set EXAMPLE_LANG=sacs

call %FUNCTIONS% :runOwnership

call %FUNCTIONS% :ownershipCheckResults >> run.log
