@echo OFF
SETLOCAL

call %FUNCTIONS% :stopOSPL

rem =======================================================
rem ==                    step_1                         ==
rem =======================================================

call %FUNCTIONS% :startOSPL

echo "=== Launching Lifecycle "
echo "=== (step 1)"
start "" /B java -jar sub/java5_Lifecycle_sub.jar > subResult_1.txt

%SLEEP2% > NUL

java -jar pub/java5_Lifecycle_pub.jar false dispose > pubResult_1.txt

%SLEEP5% > NUL

call %FUNCTIONS% :stopOSPL


rem =======================================================
rem ==                    step_2                         ==
rem =======================================================
call %FUNCTIONS% :startOSPL

echo "=== (step 2)"
start "" /B java -jar sub/java5_Lifecycle_sub.jar > subResult_2.txt
%SLEEP2% >NUL

java -jar pub/java5_Lifecycle_pub.jar false unregister > pubResult_2.txt


%SLEEP5% > NUL
call %FUNCTIONS% :stopOSPL

rem =======================================================
rem ==                    step_3                         ==
rem =======================================================
call %FUNCTIONS% :startOSPL

echo "=== (step 3)"
start "" /B java -jar sub/java5_Lifecycle_sub.jar > subResult_3.txt
%SLEEP2% > NUL

java -jar pub/java5_Lifecycle_pub.jar false unregister > pubResult_3.txt

%SLEEP5% > NUL


call %FUNCTIONS% :lifecycleCheckResults >> run.log