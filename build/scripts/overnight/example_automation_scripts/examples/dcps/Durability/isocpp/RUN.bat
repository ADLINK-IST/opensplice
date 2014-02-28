@echo OFF
rmdir /S /Q C:\tmp\pstore
del /F /Q *.txt

set EXAMPLE_LANG=isocpp

echo "===== calling runDurability with isocpp ====="

call %FUNCTIONS% :runDurabilityInit
call %FUNCTIONS% :runDurabilityISOCPP
call %FUNCTIONS% :durabilityCheckResultsISOCPP

