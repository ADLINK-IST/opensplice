@echo OFF
SETLOCAL

call %FUNCTIONS% :stopOSPL

call %FUNCTIONS% :startOSPL

echo "=== Launching WaitSet "

start "" /B java -jar sub/java5_WaitSet_sub.jar > subResult.txt

%SLEEP2% >NUL

call java -jar pub/java5_WaitSet_pub.jar > pubResult.txt

call %FUNCTIONS% :waitsetCheckResults >> run.log