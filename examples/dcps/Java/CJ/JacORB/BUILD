mkdir -p bld
cd bld
mkdir -p idlpp
cd idlpp

#
# Generate java classed from IDL
#
idlpp -C -l java ../../pingpong.idl


#
# Generate jacorb java classed from IDL
#
java -classpath $JACORB_HOME/lib/idl.jar:$JACORB_HOME/lib/logkit.jar:$CLASSPATH org.jacorb.idl.parser ../../pingpong.idl

#
# Compile generated java code
#
echo "Compile generated Java classes"
javac -classpath .:$OSPL_HOME/jar/dcpscj.jar -d .. pingpong/*.java

#
# Compile application java code
#
echo "Compile application Java classes"
cd ..
javac -classpath .:$OSPL_HOME/jar/dcpscj.jar -d . ../*.java

cd ..
