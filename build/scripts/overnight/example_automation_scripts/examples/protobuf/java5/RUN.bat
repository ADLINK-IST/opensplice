@ECHO OFF
SETLOCAL
echo "==== Calling check_osplhome===="
call %FUNCTIONS% :check_osplhome

echo "====Calling stopOSPL ===="
call %FUNCTIONS% :stopOSPL

echo "==== Calling startOSPL ===="
call %FUNCTIONS% :startOSPL

echo "==== start ProtobufSubscriber ===="
start /B java -jar sub/java5_protobuf_sub.jar > subResult.txt

echo "==== start ProtobufPublisher ===="
java -jar pub/java5_protobuf_pub.jar > pubResult.txt

echo "====Calling stopOSPL ===="
call %FUNCTIONS% :stopOSPL