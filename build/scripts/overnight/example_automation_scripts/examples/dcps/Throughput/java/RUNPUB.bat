start "" /B java -classpath "classes;%OSPL_HOME%/jar/dcpssaj.jar" publisher 1 0 1 15 > pubResult.txt 2>&1
echo %errorlevel% > pubReturn.txt 2>&1
