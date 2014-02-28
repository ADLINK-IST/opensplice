@echo OFF
SETLOCAL

echo "===== calling runContentFilteredTopic for sacpp ====="

set EXAMPLE_LANG=sacpp

call %FUNCTIONS% :runContentFilteredTopic

call %FUNCTIONS% :contentfilteredtopicCheckResult >> run.log
