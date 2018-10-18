@echo OFF
SETLOCAL

call %FUNCTIONS% :stopOSPL

call %FUNCTIONS% :startOSPL

echo "=== Launching QueryCondition "
start "" /B java -jar sub/java5_QueryCondition_sub.jar MSFT > subResult.txt
%SLEEP5% >NUL

call java -jar pub/java5_QueryCondition_pub.jar > pubResult.txt
%SLEEP5% >NUL

call %FUNCTIONS% :queryconditionCheckResults >> run.log