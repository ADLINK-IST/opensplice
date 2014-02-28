
@echo OFF
SETLOCAL

echo "===== calling runLifecycle for isocpp ====="

set EXAMPLE_LANG=isocpp

call %FUNCTIONS% :runLifecycleISOCPP

call %FUNCTIONS% :lifecycleCheckResultsISOCPP