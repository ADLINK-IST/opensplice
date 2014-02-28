@echo OFF
SETLOCAL

call %FUNCTIONS% :stopOSPL

call %FUNCTIONS% :startOSPL

echo "=== Launching Ownership "
start "" /B java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";classes OwnershipDataSubscriber > subResult.txt

%SLEEP2% >NUL
start "" /B java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";classes OwnershipDataPublisher "pub1" 5 40 1 > pub1Result.txt

%SLEEP2% >NUL
call java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";classes OwnershipDataPublisher "pub2" 10 5 0 > pub2Result.txt

call %FUNCTIONS% :ownershipCheckResults >> run.log

