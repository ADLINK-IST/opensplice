SETLOCAL

echo "===== calling HelloWorld for sacs ====="

set EXAMPLE_LANG=sacs

call %FUNCTIONS% :runHelloWorld

call %FUNCTIONS% :helloworldCheckResult >> run.log

