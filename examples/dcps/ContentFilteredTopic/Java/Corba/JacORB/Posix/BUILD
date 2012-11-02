mkdir -p bld
mkdir -p exec
cd bld
mkdir -p idlpp
mkdir -p classes
cd idlpp

#
# Generate java classes from IDL
#
echo "Compile ContentFilteredTopicData.idl"
echo "Compile ContentFilteredTopicData.idl with OpenSplice"
idlpp -C -l java ../../../../../../idl/ContentFilteredTopicData.idl

#
# Generate jacorb java classed from IDL
#
echo "Compile ContentFilteredTopicData.idl with JacORB"
$SPLICE_JAVA -classpath "$JACORB_HOME/lib/idl.jar:$JACORB_HOME/lib/endorsed/logkit.jar:$CLASSPATH" org.jacorb.idl.parser ../../../../../../idl/ContentFilteredTopicData.idl

#
# Compile generated java code
#
echo "Compile generated Java classes"
$SPLICE_JAVAC -classpath .:$OSPL_HOME/jar/dcpssaj.jar:$SPLICE_EXTRA_CP -d ../classes  ./ContentFilteredTopicData/*.java

#
# Compile application java code
#
echo "Compile application Java classes"
cd ..
pwd
$SPLICE_JAVAC -classpath .:idlpp:$OSPL_HOME/jar/dcpssaj.jar:$SPLICE_EXTRA_CP -d ./classes ../../../../src/*.java

#
# Create jar files
#
echo "Create jar files"
cd ../exec
pwd
jar cf ContentFilteredTopicDataSubscriber.jar -C ../bld/classes . 
jar cf ContentFilteredTopicDataPublisher.jar -C ../bld/classes .
cd ..
