@echo OFF
SET SLEEP5=ping 127.0.0.1 -n 5
SET SLEEP10=ping 127.0.0.1 -n 10
rem %SLEEP5% >NUL
echo "=== (step 1)"
start /B startPublisher.bat  false -action 1

%SLEEP10% >NUL

echo "=== (step 2)"
start /B startPublisher.bat  false -action 2
%SLEEP10% >NUL

echo "=== (step 3)"
start /B startPublisher.bat  false -action 3
%SLEEP10% >NUL

echo "=== (step 4)"
start /B startPublisher.bat  false -action 4
