if [ -z $OSPL_HOME ]
then
	echo "OSPL_HOME environment variable is not set"
	exit
fi
if [ -z $PROTOBUF_HOME ]
then
	echo "PROTOBUF_HOME environment variable is not set. It must point to the protobuf installation directory."
	exit
fi

protoc --version
if [ $? != 0 ]
then
   echo "Problem running protoc - cannot build protobuf example..."
   exit
fi

if [ ! -d "generated" ]; then
	mkdir generated
fi

if [ ! -d "bld" ]; then
	mkdir bld
fi
   
echo "Compiling proto files..."
echo " - ../proto/address.proto"

if [ -n "$OSPL_OUTER_HOME" ]
then
	protoc --java_out=./generated --ddsjava_out=./generated --proto_path=../../proto --proto_path=$PROTOBUF_HOME/src --proto_path=$OSPL_HOME/src/tools/protobuf/protos ../../proto/address.proto
else
	protoc --java_out=./generated --ddsjava_out=./generated --proto_path=../../proto --proto_path=$PROTOBUF_HOME/src --proto_path=$OSPL_HOME/include/protobuf ../../proto/address.proto
fi

echo "Compiling proto files done"
echo "Compiling java files... "
if [ -n "$OSPL_OUTER_HOME" ]
then
	CP_ARGS="$OSPL_HOME/jar/$SPLICE_TARGET/dcpssaj5.jar":"$OSPL_HOME/jar/$SPLICE_TARGET/dcpsprotobuf.jar"
else
	CP_ARGS="$OSPL_HOME/jar/dcpssaj5.jar":"$OSPL_HOME/jar/dcpsprotobuf.jar"
fi

SP_ARGS=generated:src

echo " - ../src/*.java"
javac -J-Xss2m -cp $CP_ARGS -sourcepath $SP_ARGS -d bld ../src/*.java

echo " - generated/address/*.java"
javac -J-Xss2m -cp $CP_ARGS -sourcepath $SP_ARGS -d bld generated/address/*.java

echo " - generated/address/dds/*.java"
javac -J-Xss2m -cp $CP_ARGS -sourcepath $SP_ARGS -d bld generated/address/dds/*.java

echo " - generated/org/omg/dds/protobuf/*.java"
javac -J-Xss2m -cp $CP_ARGS -sourcepath $SP_ARGS -d bld generated/org/omg/dds/protobuf/*.java

echo "Compiling java files done"

if [ -n "$OSPL_OUTER_HOME" ]
then
	CP="Class-Path: ../../../../../ospli/jar/$SPLICE_TARGET/dcpssaj5.jar ../../../../../ospli/jar/$SPLICE_TARGET/dcpsprotobuf.jar"
else
	CP="Class-Path: ../../../../jar/dcpssaj5.jar ../../../../jar/dcpsprotobuf.jar"
fi

printf "Creating publisher jar file... "
echo "Main-Class: ProtobufPublisher" > publisher.mf
echo $CP >> publisher.mf
jar cmf publisher.mf saj5-protobuf-publisher.jar -C bld .
rm publisher.mf
echo "Done"

printf "Creating subscriber jar file... "
echo "Main-Class: ProtobufSubscriber" > subscriber.mf
echo $CP >> subscriber.mf
jar cmf subscriber.mf saj5-protobuf-subscriber.jar -C bld .
rm subscriber.mf
echo "Done"
