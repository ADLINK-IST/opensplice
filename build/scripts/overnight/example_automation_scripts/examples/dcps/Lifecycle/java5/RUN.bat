@echo OFF
SETLOCAL

call %FUNCTIONS% :stopOSPL

rem =======================================================
rem ==                    step_1                         ==
rem =======================================================

call %FUNCTIONS% :startOSPL

echo "=== Launching Lifecycle "
echo "=== (step 1)"
start "" /B java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";classes LifecycleDataSubscriber > subResult_1.txt 

%SLEEP2% > NUL

java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";classes LifecycleDataPublisher false dispose > pubResult_1.txt

%SLEEP5% > NUL

call %FUNCTIONS% :stopOSPL


rem =======================================================
rem ==                    step_2                         ==
rem =======================================================
call %FUNCTIONS% :startOSPL

echo "=== (step 2)" 
start "" /B java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";classes LifecycleDataSubscriber > subResult_2.txt
%SLEEP2% >NUL

java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";classes LifecycleDataPublisher false unregister > pubResult_2.txt


%SLEEP5% > NUL
call %FUNCTIONS% :stopOSPL

rem =======================================================
rem ==                    step_3                         ==
rem =======================================================
call %FUNCTIONS% :startOSPL

echo "=== (step 3)" 
start "" /B java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";classes LifecycleDataSubscriber > subResult_3.txt
%SLEEP2% > NUL

java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";classes LifecycleDataPublisher false unregister > pubResult_3.txt

%SLEEP5% > NUL 


call %FUNCTIONS% :lifecycleCheckResults >> run.log