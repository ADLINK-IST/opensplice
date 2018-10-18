@echo OFF
SETLOCAL

echo "==== Calling check_osplhome===="
call %FUNCTIONS% :check_osplhome

echo "====Calling stopOSPL ===="
call %FUNCTIONS% :stopOSPL

echo "====Calling startOSPL ===="
call %FUNCTIONS% :startOSPL

echo "=== Launching ContentFilteredTopic "

start "" /B java -jar sub/java5_ContentFilteredTopic_sub.jar GE > subResult.txt

%SLEEP5% >NUL

java -jar pub/java5_ContentFilteredTopic_pub.jar > pubResult.txt

%SLEEP5% > NUL

echo "===== Calling contentfilteredtopicCheckResult ===="
call %FUNCTIONS% :contentfilteredtopicCheckResult >> run.log

echo "===== calling stopOSPL ===="
rem Don't kill it too soon.
call %FUNCTIONS% :stopOSPL