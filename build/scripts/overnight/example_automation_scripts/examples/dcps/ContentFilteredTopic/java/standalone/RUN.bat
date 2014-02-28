@echo OFF
SETLOCAL

echo "==== Calling check_osplhome===="
call %FUNCTIONS% :check_osplhome

echo "====Calling stopOSPL ===="
call %FUNCTIONS% :stopOSPL

echo "====Calling startOSPL ===="
call %FUNCTIONS% :startOSPL

echo "=== Launching ContentFilteredTopic "

start "" /B java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";classes ContentFilteredTopicDataSubscriber GE > subResult.txt

%SLEEP5% >NUL

java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";classes ContentFilteredTopicDataPublisher > pubResult.txt

%SLEEP5% > NUL

echo "===== Calling contentfilteredtopicCheckResult ===="
call %FUNCTIONS% :contentfilteredtopicCheckResult >> run.log

echo "===== calling stopOSPL ===="
rem Don't kill it too soon.
call %FUNCTIONS% :stopOSPL