@ECHO OFF
SETLOCAL
echo "==== Calling check_osplhome===="
call %FUNCTIONS% :check_osplhome

echo "====Calling stopOSPL ===="
call %FUNCTIONS% :stopOSPL

echo "==== Calling startOSPL ===="
call %FUNCTIONS% :startOSPL

echo "==== start ProtobufSubscriber ===="
start "" /B subscriber.exe > subResult.txt

echo "==== start ProtobufPublisher ===="
publisher.exe > pubResult.txt

echo "====Calling stopOSPL ===="
call %FUNCTIONS% :stopOSPL
