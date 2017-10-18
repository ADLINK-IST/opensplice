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
if [ -z $PROTOBUF_LIB_HOME ]
then
        echo "PROTOBUF_LIB_HOME variable not set. Looking for library in default locations."
else
        LDFLAGS=-L$PROTOBUF_LIB_HOME
fi

# Based on checkconf script
TARGET_SHORT=`uname`
if [ "$TARGET_SHORT" = "Darwin" ]
then
        CPP11_FLAG="-std=c++11 -DOSPL_USE_CXX11"
        CXX=clang++
else
        CXX=g++
        GCC_FULLVERSION=`gcc -dumpversion`
        GCC_SUPPORTS_CPLUSPLUS11=`echo $GCC_FULLVERSION | gawk '{ print $1>="4.3"?"1":"0"}'`
        GDESC=./generated/google/protobuf/descriptor.pb.cc
        if [ "$GCC_SUPPORTS_CPLUSPLUS11" = "1" ]
        then
                CPP11_FLAG="-std=c++0x"
        else
                CPP11_FLAG=""
        fi
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

echo "Compiling proto files..."
echo " - ../proto/address.proto"

if [ -n "$OSPL_OUTER_HOME" ]
then
	protoc --cpp_out=./generated --ddscpp_out=./generated --proto_path=../../proto --proto_path=$PROTOBUF_HOME/src --proto_path=$OSPL_HOME/src/tools/protobuf/protos ../../proto/address.proto
else
	protoc --cpp_out=./generated --ddscpp_out=./generated --proto_path=../../proto --proto_path=$PROTOBUF_HOME/src --proto_path=$OSPL_HOME/include/protobuf ../../proto/address.proto
fi

if [ "$TARGET_SHORT" != "Darwin" ]
then
   echo " - $PROTOBUF_HOME/src/google/protobuf/descriptor.proto"
   protoc --cpp_out=./generated --proto_path="$PROTOBUF_HOME/src" "$PROTOBUF_HOME/src/google/protobuf/descriptor.proto"
fi

echo " - $OSPL_HOME/src/tools/protobuf/protos/omg/dds/descriptor.proto"

if [ -n "$OSPL_OUTER_HOME" ]
then
	protoc --cpp_out=./generated --proto_path="$PROTOBUF_HOME/src" --proto_path="$OSPL_HOME/src/tools/protobuf/protos" "$OSPL_HOME/src/tools/protobuf/protos/omg/dds/descriptor.proto"
else
	protoc --cpp_out=./generated --proto_path="$PROTOBUF_HOME/src" --proto_path="$OSPL_HOME/include/protobuf" "$OSPL_HOME/include/protobuf/omg/dds/descriptor.proto"
fi

echo "Compiling proto files done"

echo "Compiling publisher..."
if [ -n "$OSPL_OUTER_HOME" ]
then
	$CXX -g $CPP11_FLAG -I$PROTOBUF_HOME/src -I./generated -I$OSPL_HOME/src/api/dcps/c++/common/include -I$OSPL_HOME/src/api/dcps/common/include -I$OSPL_HOME/src/api/dcps/c++/sacpp/include -I$OSPL_HOME/src/api/dcps/c++/sacpp/common/include -I$OSPL_HOME/src/abstraction/os/include -I$OSPL_HOME/src/database/database/include -I$OSPL_HOME/src/kernel/include -I$OSPL_HOME/src/osplcore/bld/$SPLICE_TARGET -I$OSPL_HOME/src/user/include -I../../.. -I$OSPL_HOME/src/api/dcps/isocpp2/include ../src/implementation.cpp ../src/publisher.cpp ./generated/address.cpp ./generated/address.pb.cc ./generated/addressSplDcps.cpp ./generated/ospl_protobuf_commonSplDcps.cpp ./generated/ospl_protobuf_common.cpp $GDESC ./generated/omg/dds/descriptor.pb.cc -L$OSPL_HOME/lib/$SPLICE_TARGET $LDFLAGS -ldcpsisocpp2 -lddskernel -lprotobuf -o publisher
else
	$CXX -g $CPP11_FLAG -I$PROTOBUF_HOME/src -I./generated -I$OSPL_HOME/include/dcps/C++/SACPP -I$OSPL_HOME/include/dcps/C++/isocpp2 -I$OSPL_HOME/include -I$OSPL_HOME/include/sys -I../../.. ../src/implementation.cpp ../src/publisher.cpp ./generated/address.cpp ./generated/address.pb.cc ./generated/addressSplDcps.cpp ./generated/ospl_protobuf_commonSplDcps.cpp ./generated/ospl_protobuf_common.cpp $GDESC ./generated/omg/dds/descriptor.pb.cc -L$OSPL_HOME/lib $LDFLAGS -ldcpsisocpp2 -lddskernel -lprotobuf -o publisher
fi

echo "Compiling publisher done"

echo "Compiling subscriber..."
if [ -n "$OSPL_OUTER_HOME" ]
then
	$CXX -g $CPP11_FLAG -I$PROTOBUF_HOME/src -I./generated -I$OSPL_HOME/src/api/dcps/c++/common/include -I$OSPL_HOME/src/api/dcps/common/include -I$OSPL_HOME/src/api/dcps/c++/sacpp/include -I$OSPL_HOME/src/api/dcps/c++/sacpp/common/include -I$OSPL_HOME/src/abstraction/os/include -I$OSPL_HOME/src/database/database/include -I$OSPL_HOME/src/kernel/include -I$OSPL_HOME/src/osplcore/bld/$SPLICE_TARGET -I$OSPL_HOME/src/user/include -I../../.. -I$OSPL_HOME/src/api/dcps/isocpp2/include ../src/implementation.cpp ../src/subscriber.cpp ./generated/address.cpp ./generated/address.pb.cc ./generated/addressSplDcps.cpp ./generated/ospl_protobuf_commonSplDcps.cpp ./generated/ospl_protobuf_common.cpp $GDESC ./generated/omg/dds/descriptor.pb.cc -L$OSPL_HOME/lib/$SPLICE_TARGET $LDFLAGS -ldcpsisocpp2 -lddskernel -lprotobuf -o subscriber
else
	$CXX -g $CPP11_FLAG -I$PROTOBUF_HOME/src -I./generated -I$OSPL_HOME/include/dcps/C++/SACPP -I$OSPL_HOME/include/dcps/C++/isocpp2 -I$OSPL_HOME/include -I$OSPL_HOME/include/sys -I../../.. ../src/implementation.cpp ../src/subscriber.cpp ./generated/address.cpp ./generated/address.pb.cc ./generated/addressSplDcps.cpp ./generated/ospl_protobuf_commonSplDcps.cpp ./generated/ospl_protobuf_common.cpp $GDESC ./generated/omg/dds/descriptor.pb.cc -L$OSPL_HOME/lib $LDFLAGS -ldcpsisocpp2 -lddskernel -lprotobuf -o subscriber
fi

echo "Compiling subscriber done"
