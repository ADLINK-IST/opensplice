@echo OFF
SETLOCAL

echo "===== calling HelloWorld for sacpp ====="

set EXAMPLE_LANG=sacpp

call %FUNCTIONS% :runHelloWorld

call %FUNCTIONS% :helloworldCheckResult >> run.log
