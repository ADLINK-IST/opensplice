@echo OFF
SETLOCAL

echo "==== Calling check_osplhome===="
call %FUNCTIONS% :check_osplhome

echo "====Calling stopOSPL ===="
call %FUNCTIONS% :stopOSPL

@echo off
set EXPECTED_RESULT=..\..\expected_results
set WIN_BATCH="..\..\..\..\win_batch"


echo "==== Calling startOSPL ===="
call %FUNCTIONS% :startOSPL

echo "==== Starting java subscriber ====="
java -jar java5_BuiltInTopics.jar > subResult.txt
%SLEEP5% > NUL

echo "===== Calling builtintopicsCheckResult ===="
call %FUNCTIONS% :builtintopicsCheckResult >> run.log

echo "===== calling stopOSPL ===="
rem Don't kill it too soon.
call %FUNCTIONS% :stopOSPL


