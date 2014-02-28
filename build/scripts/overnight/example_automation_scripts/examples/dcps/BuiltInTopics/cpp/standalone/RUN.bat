@echo OFF
SETLOCAL

echo "===== calling runBuiltInTopics for sacpp ====="

set EXAMPLE_LANG=sacpp

call %FUNCTIONS% :runBuiltInTopics

call %FUNCTIONS% :builtintopicsCheckResult >> run.log

