@echo OFF
SETLOCAL

call %FUNCTIONS% :stopOSPL

call %FUNCTIONS% :startOSPL

echo "== Launching Listener "
start "" /B java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";classes ListenerDataSubscriber > subResult.txt
%SLEEP2% >NUL

java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";classes ListenerDataPublisher > pubResult.txt

call %FUNCTIONS% :listenerCheckResults >> run.log
