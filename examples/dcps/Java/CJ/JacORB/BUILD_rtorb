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
$SPLICE_JAVA -classpath "$RTORB_HOME/lib/idl.jar:$RTORB_HOME/lib/logkit.jar:$CLASSPATH" org.jacorb.idl.parser ../../pingpong.idl

#
# Compile generated java code
#
echo "Compile generated Java classes"
$SPLICE_JAVAC -classpath .:$OSPL_HOME/jar/dcpscj.jar:$SPLICE_EXTRA_CP -d .. pingpong/*.java

#
# Compile application java code
#
echo "Compile application Java classes"
cd ..
$SPLICE_JAVAC -classpath .:$OSPL_HOME/jar/dcpscj.jar:$SPLICE_EXTRA_CP -d . ../*.java

cd ..
