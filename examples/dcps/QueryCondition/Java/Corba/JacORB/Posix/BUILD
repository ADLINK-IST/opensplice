mkdir -p exec
mkdir -p bld
cd bld
mkdir -p idlpp
cd idlpp

#
# Generate java classes from IDL
#
echo "Compile QueryConditionData.idl with OpenSplice"
idlpp -C -l java ../../../../../../idl/QueryConditionData.idl

#
# Generate jacorb java classed from IDL
#
echo "Compile QueryConditionData.idl with JacORB"
$SPLICE_JAVA -classpath "$JACORB_HOME/lib/idl.jar:$JACORB_HOME/lib/endorsed/logkit.jar:$CLASSPATH" org.jacorb.idl.parser ../../../../../../idl/QueryConditionData.idl

#
# Compile generated java code
#
echo "Compile generated Java classes"
$SPLICE_JAVAC -classpath .:$OSPL_HOME/jar/dcpssaj.jar:$SPLICE_EXTRA_CP -d .. QueryConditionData/*.java

#
# Compile application java code
#
echo "Compile application Java classes"
cd ..
pwd
$SPLICE_JAVAC -classpath .:$OSPL_HOME/jar/dcpssaj.jar:$SPLICE_EXTRA_CP -d ./ ../../../../src/*.java
jar cf ../exec/QueryConditionDataSubscriber.jar *.class QueryConditionData/*.class
jar cf ../exec/QueryConditionDataPublisher.jar *.class QueryConditionData/*.class
cd ..

