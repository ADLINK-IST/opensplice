start "" /B  java -classpath "%OSPL_HOME%\jar\ddsface.jar";classes HelloWorldDataSubscriber > subResult.txt
echo %errorlevel% > subReturn.txt 2>&1
