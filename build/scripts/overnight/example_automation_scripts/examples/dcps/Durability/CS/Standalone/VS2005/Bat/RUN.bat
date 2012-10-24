rem Scenario 3.3 (cf README.html)
ospl stop
set wait=10
ping 127.0.0.1 -n %wait% > NUL
ospl start
set wait=10
ping 127.0.0.1 -n %wait% > NUL
call startPublisher.bat persistent false
rem 2 seconds
set wait=2
ping 127.0.0.1 -n %wait% > NUL
call startSubscriber.bat persistent
ospl stop
set wait=10
ping 127.0.0.1 -n %wait% > NUL
ospl start
set wait=10
ping 127.0.0.1 -n %wait% > NUL
call startSubscriber.bat persistent
