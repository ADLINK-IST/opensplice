@echo OFF
SETLOCAL

echo "===== calling runBuiltInTopics for isocpp ====="

set EXAMPLE_LANG=isocpp

call %FUNCTIONS% :runBuiltInTopicsISOCPP

call %FUNCTIONS% :builtintopicsCheckResult >> run.log

