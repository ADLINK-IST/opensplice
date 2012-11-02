
mkdir -p exec
mkdir -p bld
cd bld
mkdir -p idlpp
cd idlpp

#
# Generate java classes from IDL
#
echo "Compile OwnershipData.idl with OpenSplice"
idlpp -C -l java ../../../../../../idl/OwnershipData.idl

#
# Generate jacorb java classed from IDL
#
echo "Compile OwnershipData.idl with JacORB"
$SPLICE_JAVA -classpath "$JACORB_HOME/lib/idl.jar:$JACORB_HOME/lib/endorsed/logkit.jar:$CLASSPATH" org.jacorb.idl.parser ../../../../../../idl/OwnershipData.idl

#
# Compile generated java code
#
echo "Compile generated Java classes"
$SPLICE_JAVAC -classpath .:$OSPL_HOME/jar/dcpssaj.jar:$SPLICE_EXTRA_CP -d .. OwnershipData/*.java

#
# Compile application java code
#
echo "Compile application Java classes"
cd ..
echo "Curren dir is $PWD"

$SPLICE_JAVAC -classpath .:$OSPL_HOME/jar/dcpssaj.jar:$SPLICE_EXTRA_CP -d ./ ../../../../src/*.java
jar cf ../exec/OwnershipDataSubscriber.jar *.class OwnershipData/*.class
jar cf ../exec/OwnershipDataPublisher.jar *.class OwnershipData/*.class
cd ..

