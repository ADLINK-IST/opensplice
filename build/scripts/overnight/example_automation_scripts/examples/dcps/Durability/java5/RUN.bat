@echo OFF
IF EXIST C:\tmp\pstore rmdir /S /Q C:\tmp\pstore
IF EXIST .\tmp\pstore rmdir /S /Q .\tmp\pstore

call %FUNCTIONS% :runDurabilityInit

echo "=== Launching Durability "
echo "=== Scenario 3.1"
echo "=== running the Subscriber"
start "DurabilityExample" /B java -jar sub/java5_Durability_sub.jar transient > subResult_3_1.txt

%SLEEP5% > NUL

echo "=== running the Publisher"
start "" /B java -jar pub/java5_Durability_pub.jar transient true true > pubResult_3_1.txt

REM Wait 30s to allow the publisher to complete and terminate rather than kill it
%SLEEP30% > NUL

call %FUNCTIONS% :stopOSPL

call %FUNCTIONS% :startOSPL

%SLEEP2% >NUL

echo "=== Scenario 3.2"
echo "=== running a first Subscriber"
start "" /B java -jar sub/java5_Durability_sub.jar transient > subResult_3_2_1.txt

%SLEEP2% >NUL

echo "=== running the Publisher"
start "" /B java -jar pub/java5_Durability_pub.jar transient false true > pubResult_3_2.txt

%SLEEP5% >NUL

echo "=== running a second Subscriber"
start "" /B java -jar sub/java5_Durability_sub.jar transient > subResult_3_2_2.txt

REM Wait 30s to allow the publisher to complete and terminate rather than kill it
%SLEEP30% >NUL

echo "=== Scenario 3.3"
echo "=== Stop OpenSplice"

call %FUNCTIONS% :stopOSPL

call %FUNCTIONS% :startOSPL

echo "=== running a first Subscriber"
start "" /B java -jar sub/java5_Durability_sub.jar persistent > subResult_3_3_1.txt

%SLEEP2% >NUL
echo "=== running the Publisher"
start "" /B java -jar pub/java5_Durability_pub.jar persistent false true > pubResult_3_3.txt

REM Wait 30s to allow the publisher to complete and terminate rather than kill it
%SLEEP30% >NUL

call %FUNCTIONS% :stopOSPL

call %FUNCTIONS% :startOSPL

echo "=== running a second Subscriber after stop/start of OpenSplice"
start "" /B java -jar sub/java5_Durability_sub.jar persistent > subResult_3_3_2.txt

%SLEEP10% >NUL

call %FUNCTIONS% :durabilityCheckResultsJava5 >> run.log

