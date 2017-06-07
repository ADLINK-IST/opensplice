echo OFF
SETLOCAL

echo "===== calling Ownership for isocpp ====="

set EXAMPLE_LANG=isocpp

call %FUNCTIONS% :runOwnershipISOCPP

call %FUNCTIONS% :ownershipCheckResults
