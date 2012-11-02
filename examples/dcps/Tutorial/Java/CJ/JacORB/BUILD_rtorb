mkdir bld
cd bld
mkdir idlpp
cd idlpp

#
# Generate java classes from IDL
#
idlpp -C -l java ../../Chat.idl

#
# Generate jacorb java classed from IDL
#
$SPLICE_JAVA -classpath "$RTORB_HOME/lib/idl.jar:$RTORB_HOME/lib/logkit.jar:$CLASSPATH" org.jacorb.idl.parser ../../Chat.idl

#
# Compile generated java code
#
echo "Compile generated Java classes"
$SPLICE_JAVAC -classpath .:$OSPL_HOME/jar/dcpscj.jar:$SPLICE_EXTRA_CP -d .. Chat/*.java

#
# Compile application java code
#
echo "Compile application Java classes"
cd ..
$SPLICE_JAVAC -classpath .:$OSPL_HOME/jar/dcpscj.jar:$SPLICE_EXTRA_CP -d . ../chatroom/*.java

cd ..
