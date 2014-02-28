@echo OFF
SETLOCAL

echo "===== calling HelloWorld for sac ====="

set EXAMPLE_LANG=sac

call %FUNCTIONS% :runHelloWorld

call %FUNCTIONS% :helloworldCheckResult >> run.log
