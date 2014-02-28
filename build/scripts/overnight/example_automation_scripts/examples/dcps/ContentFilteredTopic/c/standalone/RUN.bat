
@echo OFF
SETLOCAL

echo "===== calling runContentFilteredTopic for sac ====="

set EXAMPLE_LANG=sac

call %FUNCTIONS% :runContentFilteredTopic

call %FUNCTIONS% :contentfilteredtopicCheckResult >> run.log