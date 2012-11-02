mkdir -p bld
mkdir -p exec
cd bld
mkdir -p idlpp
mkdir -p classes
cd idlpp

#
# Generate java classes from IDL
#
echo "Compile ListenerData.idl"
idlpp -S -l java ../../../../../idl/ListenerData.idl

#
# Compile generated java code
#
echo "Compile generated Java classes"
$SPLICE_JAVAC -classpath .:$OSPL_HOME/jar/dcpssaj.jar:$SPLICE_EXTRA_CP -d ../classes ./ListenerData/*.java

#
# Compile application java code
#
echo "Compile application Java classes"
cd ..
pwd
$SPLICE_JAVAC -classpath .:idlpp:$OSPL_HOME/jar/dcpssaj.jar:$SPLICE_EXTRA_CP -d ./classes ../../../src/*.java

#
# Create jar files
#
echo "Create jar files"
cd ../exec
pwd
jar cf ListenerDataSubscriber.jar -C ../bld/classes . 
jar cf ListenerDataPublisher.jar -C ../bld/classes .
cd ..
