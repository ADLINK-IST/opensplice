SETLOCAL
@ECHO OFF

echo "===== calling QueryCondition for sacs ====="

set EXAMPLE_LANG=sacs

call %FUNCTIONS% :runQueryCondition

call %FUNCTIONS% :queryconditionCheckResults >> run.log
