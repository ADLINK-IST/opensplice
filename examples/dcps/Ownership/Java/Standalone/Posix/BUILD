
mkdir -p exec
mkdir -p bld
cd bld
mkdir -p idlpp
cd idlpp

#
# Generate java classes from IDL
#
idlpp -S -l java ../../../../../idl/OwnershipData.idl

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

$SPLICE_JAVAC -classpath .:$OSPL_HOME/jar/dcpssaj.jar:$SPLICE_EXTRA_CP -d ./ ../../../src/*.java
jar cf ../exec/OwnershipDataSubscriber.jar *.class OwnershipData/*.class
jar cf ../exec/OwnershipDataPublisher.jar *.class OwnershipData/*.class
cd ..

