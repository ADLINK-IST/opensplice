@echo OFF
SETLOCAL
echo "===== calling runBuiltInTopics for sacs ====="

set EXAMPLE_LANG=sacs

call %FUNCTIONS% :runBuiltInTopics

call %FUNCTIONS% :builtintopicsCheckResult >> run.log


