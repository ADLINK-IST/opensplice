@echo OFF
SETLOCAL

echo "==== Calling check_osplhome===="
call %FUNCTIONS% :check_osplhome

echo "====Calling stopOSPL ===="
call %FUNCTIONS% :stopOSPL

echo "====Calling startOSPL ===="
call %FUNCTIONS% :startOSPL

echo "=== Launching Java CORBA HelloWorld "

start "" /B java  -Djava.endorsed.dirs="%JACORB_HOME%\lib\endorsed" -classpath "%OSPL_HOME%\jar\dcpscj.jar";classes HelloWorldDataSubscriber > subResult.txt
%SLEEP5% >NUL

java -Djava.endorsed.dirs="%JACORB_HOME%\lib\endorsed" -classpath "%OSPL_HOME%\jar\dcpscj.jar";classes HelloWorldDataPublisher > pubResult.txt

%SLEEP5% >NUL
 
call %FUNCTIONS% :helloworldCheckResult >> run.log

call %FUNCTIONS% :stopOSPL