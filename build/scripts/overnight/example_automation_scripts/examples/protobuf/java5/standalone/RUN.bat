@ECHO OFF
SETLOCAL
echo "==== Calling check_osplhome===="
call %FUNCTIONS% :check_osplhome

echo "====Calling stopOSPL ===="
call %FUNCTIONS% :stopOSPL

echo "==== Calling startOSPL ===="
call %FUNCTIONS% :startOSPL

echo "==== start ProtobufSubscriber ===="
start /B java -cp saj5-protobuf-subscriber.jar;..\..\..\..\jar\dcpssaj5.jar;..\..\..\..\jar\dcpsprotobuf.jar ProtobufSubscriber > subResult.txt

echo "==== start ProtobufPublisher ===="
java -cp saj5-protobuf-publisher.jar;..\..\..\..\jar\dcpssaj5.jar;..\..\..\..\jar\dcpsprotobuf.jar ProtobufPublisher > pubResult.txt

echo "====Calling stopOSPL ===="
call %FUNCTIONS% :stopOSPL