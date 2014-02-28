@echo OFF
SETLOCAL

echo "===== calling runBuiltInTopics for sac ====="

set EXAMPLE_LANG=sac

call %FUNCTIONS% :runBuiltInTopics

call %FUNCTIONS% :builtintopicsCheckResult >> run.log

