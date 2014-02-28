@echo OFF
SETLOCAL

call %FUNCTIONS% :stopOSPL

call %FUNCTIONS% :startOSPL

echo "=== Launching WaitSet "

start "" /B java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";classes WaitSetDataSubscriber > subResult.txt

%SLEEP2% >NUL

call java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";classes WaitSetDataPublisher > pubResult.txt

call %FUNCTIONS% :waitsetCheckResults >> run.log