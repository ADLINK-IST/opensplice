start "" /B  java -classpath "%OSPL_HOME%\jar\dcpssaj5.jar";classes HelloWorldDataSubscriber > subResult.txt
echo %errorlevel% > subReturn.txt 2>&1
