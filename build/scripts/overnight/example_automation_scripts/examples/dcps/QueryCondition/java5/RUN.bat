@echo OFF
SETLOCAL

call %FUNCTIONS% :stopOSPL

call %FUNCTIONS% :startOSPL

echo "=== Launching QueryCondition "
start "" /B java -classpath "%OSPL_HOME%\jar\dcpssaj5.jar";classes QueryConditionDataSubscriber MSFT > subResult.txt
%SLEEP5% >NUL

call java -classpath "%OSPL_HOME%\jar\dcpssaj5.jar";classes QueryConditionDataPublisher > pubResult.txt
%SLEEP5% >NUL

call %FUNCTIONS% :queryconditionCheckResults >> run.log