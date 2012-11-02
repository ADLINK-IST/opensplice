mkdir -p exec
mkdir -p bld
cd bld
mkdir -p idlpp
cd idlpp

#
# Generate java classes from IDL
#
idlpp -S -l java ../../../../../idl/WaitSetData.idl

#
# Compile generated java code
#
echo "Compile generated Java classes"
$SPLICE_JAVAC -classpath .:$OSPL_HOME/jar/dcpssaj.jar:$SPLICE_EXTRA_CP -d .. WaitSetData/*.java

#
# Compile application java code
#
echo "Compile application Java classes"
cd ..
pwd
$SPLICE_JAVAC -classpath .:$OSPL_HOME/jar/dcpssaj.jar:$SPLICE_EXTRA_CP -d ./ ../../../src/*.java
jar cf ../exec/WaitSetDataSubscriber.jar  ./*.class WaitSetData/*.class
jar cf ../exec/WaitSetDataPublisher.jar  ./*.class WaitSetData/*.class
cd ..
cd .
