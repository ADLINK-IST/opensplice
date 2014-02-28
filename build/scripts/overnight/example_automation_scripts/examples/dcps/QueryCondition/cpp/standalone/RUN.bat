@echo OFF
SETLOCAL

echo "===== calling Ownership for sacpp ====="

set EXAMPLE_LANG=sacpp

call %FUNCTIONS% :runQueryCondition

call %FUNCTIONS% :queryconditionCheckResults >> run.log
