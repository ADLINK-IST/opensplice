@echo OFF
SETLOCAL

echo "==== Calling check_osplhome===="
call %FUNCTIONS% :check_osplhome

echo "====Calling stopOSPL ===="
call %FUNCTIONS% :stopOSPL

echo "====Calling startOSPL ===="
call %FUNCTIONS% :startOSPL

echo "=== Launching java HelloWorld "
start "" /B  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";classes HelloWorldDataSubscriber > subResult.txt

%SLEEP5% >NUL

java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";classes HelloWorldDataPublisher > pubResult.txt

%SLEEP5% >NUL

call %FUNCTIONS% :helloworldCheckResult >> run.log

call %FUNCTIONS% :stopOSPL