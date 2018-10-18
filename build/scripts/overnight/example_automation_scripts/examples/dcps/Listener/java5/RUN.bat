@echo OFF
SETLOCAL

call %FUNCTIONS% :stopOSPL

call %FUNCTIONS% :startOSPL

echo "== Launching Listener "
start "" /B java -jar sub/java5_Listener_sub.jar > subResult.txt
%SLEEP2% >NUL

java -jar pub/java5_Listener_pub.jar > pubResult.txt

call %FUNCTIONS% :listenerCheckResults >> run.log
