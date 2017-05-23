@echo OFF
IF EXIST C:\tmp\pstore rmdir /S /Q C:\tmp\pstore
IF EXIST .\tmp\pstore rmdir /S /Q .\tmp\pstore

del /F /Q *.txt

set EXAMPLE_LANG=isocpp

echo "===== calling runDurability with isocpp ====="

call %FUNCTIONS% :runDurabilityInit
call %FUNCTIONS% :runDurabilityISOCPP
call %FUNCTIONS% :durabilityCheckResultsISOCPP >> run.log

