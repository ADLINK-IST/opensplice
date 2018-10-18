@echo OFF
SETLOCAL

call %FUNCTIONS% :stopOSPL

call %FUNCTIONS% :startOSPL

echo "=== Launching Ownership "
start "" /B java -jar sub/java5_Ownership_sub.jar > subResult.txt

%SLEEP2% >NUL
start "" /B java -jar pub/java5_Ownership_pub.jar "pub1" 5 40 1 > pub1Result.txt

%SLEEP2% >NUL
call java -jar pub/java5_Ownership_pub.jar "pub2" 10 5 0 > pub2Result.txt

call %FUNCTIONS% :ownershipCheckResults >> run.log

