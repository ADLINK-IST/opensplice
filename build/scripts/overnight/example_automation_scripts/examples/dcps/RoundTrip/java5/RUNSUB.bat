start "" /B java -classpath "classes;%OSPL_HOME%/jar/dcpssaj5.jar" pong %SUB_PARAMS% > pongResult.txt 2>&1
echo %errorlevel% > subReturn.txt 2>&1
