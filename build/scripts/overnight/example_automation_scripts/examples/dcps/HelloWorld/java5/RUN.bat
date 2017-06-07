@echo OFF
SETLOCAL

set NAME="HelloWorld - java5"

echo "==== Calling check_osplhome===="
call %FUNCTIONS% :check_osplhome

echo "====Calling stopOSPL ===="
call %FUNCTIONS% :stopOSPL

echo "====Calling startOSPL ===="
call %FUNCTIONS% :startOSPL

echo "=== Launching java HelloWorld "
start CMD /C RUNSUB.bat

%SLEEP5% >NUL

java -classpath "%OSPL_HOME%\jar\dcpssaj5.jar";classes HelloWorldDataPublisher > pubResult.txt
set PUB_RESULT=%errorlevel%
set /p SUB_RESULT=<subReturn.txt

%SLEEP5% >NUL

call %FUNCTIONS% :checkResultZero >> run.log

call %FUNCTIONS% :stopOSPL