@echo OFF
rmdir /S /Q C:\tmp\pstore
del /F /Q *.log
del /F /Q *.txt

set EXAMPLE_LANG=sacs

echo "===== calling runDurability with sacs ====="

call %FUNCTIONS% :runDurabilityInit
call %FUNCTIONS% :runDurability
call %FUNCTIONS% :durabilityCheckResults