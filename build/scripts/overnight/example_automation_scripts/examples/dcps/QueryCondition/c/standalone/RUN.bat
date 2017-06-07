@echo OFF
SETLOCAL

echo "===== calling QueryCondition for sac ====="

set EXAMPLE_LANG=sac

call %FUNCTIONS% :runQueryCondition

call %FUNCTIONS% :queryconditionCheckResults >> run.log
