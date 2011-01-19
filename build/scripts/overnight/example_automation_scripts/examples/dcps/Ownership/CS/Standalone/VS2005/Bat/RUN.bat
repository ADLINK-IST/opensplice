call startSubscriber.bat
rem 2 seconds
set wait=2
ping 127.0.0.1 -n %wait% > NUL
call startPublishers.bat