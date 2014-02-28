@echo OFF
SETLOCAL

echo "===== calling runContentFilteredTopic for sacs ====="

set EXAMPLE_LANG=sacs

call %FUNCTIONS% :runContentFilteredTopic

call %FUNCTIONS% :contentfilteredtopicCheckResult >> run.log
