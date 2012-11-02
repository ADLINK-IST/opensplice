@echo OFF

rem setting variables 
SET SLEEP2=ping 127.0.0.1 -n 2
SET SLEEP5=ping 127.0.0.1 -n 5
SET SLEEP15=ping 127.0.0.1 -n 15
SET SLEEP10=ping 127.0.0.1 -n 10

echo "=== Scenario 3.1"
start /B startPublisher.bat transient true true
rem %SLEEP2% >NUL

del /F subResult_3.1.txt
call startSubscriber.bat transient subResult_3.1.txt

echo "=== Scenario 3.2" 
start /B startPublisher.bat transient false true
%SLEEP2% >NUL
rem echo === stopping publisher >> pubResult_3.2.txt

echo "=== running a first Subscriber"   > subResult_3.2.1.txt
call startSubscriber.bat transient subResult_3.2.1.txt

echo "=== running a second Subscriber" > subResult_3.2.2.txt
call startSubscriber.bat transient subResult_3.2.2.txt

echo "=== Scenario 3.3"
echo "=== Stop OpenSplice"
%VG_OSPL_START% ospl stop
%SLEEP2% >NUL
echo "=== Start OpenSplice"
%VG_OSPL_START% ospl start
%VG_START_SLEEP%
start /B startPublisher.bat persistent false true
%SLEEP2% >NUL

echo "=== running a first Subscriber" > subResult_3.3.1.txt
call startSubscriber.bat persistent subResult_3.3.1.txt
%SLEEP15% >NUL

echo "=== Stop OpenSplice"
%VG_OSPL_START% ospl stop
echo "=== waiting 10s to let OpenSplice finish stopping"
%SLEEP15% >NUL

echo "=== Start OpenSplice"
%VG_OSPL_START% ospl start

%VG_START_SLEEP%
%SLEEP5% >NUL
echo "=== running a second Subscriber after stop/start of OpenSplice" > subResult_3.3.2.txt
call startSubscriber.bat persistent subResult_3.3.2.txt
