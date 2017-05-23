@echo OFF
IF EXIST C:\tmp\pstore rmdir /S /Q C:\tmp\pstore
IF EXIST .\tmp\pstore rmdir /S /Q .\tmp\pstore

set EXAMPLE_LANG=sac

echo "===== calling runDurability with sac ====="

call %FUNCTIONS% :runDurabilityInit
call %FUNCTIONS% :runDurability
call %FUNCTIONS% :durabilityCheckResults >> run.log
