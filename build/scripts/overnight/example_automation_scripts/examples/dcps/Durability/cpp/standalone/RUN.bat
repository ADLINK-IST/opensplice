@echo OFF
rmdir /S /Q C:\tmp\pstore

set EXAMPLE_LANG=sacpp

echo "===== calling runDurability with sacpp ====="

call %FUNCTIONS% :runDurabilityInit
call %FUNCTIONS% :runDurability
call %FUNCTIONS% :durabilityCheckResults >> run.log

