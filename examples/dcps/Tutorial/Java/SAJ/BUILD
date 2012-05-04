mkdir bld
cd bld
mkdir idlpp
cd idlpp

#
# Generate java classes from IDL
#
idlpp -S -l java ../../Chat.idl

#
# Compile generated java code
#
echo "Compile generated Java classes"
$SPLICE_JAVAC -classpath .:$OSPL_HOME/jar/dcpssaj.jar:$SPLICE_EXTRA_CP -d .. Chat/*.java

#
# Compile application java code
#
echo "Compile application Java classes"
cd ..
$SPLICE_JAVAC -classpath .:$OSPL_HOME/jar/dcpssaj.jar:$SPLICE_EXTRA_CP -d . ../chatroom/*.java 

cd ..
