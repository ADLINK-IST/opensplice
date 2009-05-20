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
javac -classpath .:$OSPL_HOME/jar/dcpssaj.jar -d .. Chat/*.java

#
# Compile application java code
#
echo "Compile application Java classes"
cd ..
javac -classpath .:$OSPL_HOME/jar/dcpssaj.jar -d . ../chatroom/*.java 

cd ..
