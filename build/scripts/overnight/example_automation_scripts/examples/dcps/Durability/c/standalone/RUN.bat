@echo OFF
rmdir /S /Q C:\tmp\pstore
set EXAMPLE_LANG=sac

echo "===== calling runDurability with sac ====="

call %FUNCTIONS% :runDurabilityInit
call %FUNCTIONS% :runDurability
call %FUNCTIONS% :durabilityCheckResults >> run.log
