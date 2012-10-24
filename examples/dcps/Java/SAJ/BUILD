mkdir -p bld
cd bld
mkdir -p idlpp
cd idlpp

#
# Generate java classed from IDL
#
idlpp -S -l java ../../pingpong.idl

#
# Compile generated java code
#
echo "Compile generated Java classes"
$SPLICE_JAVAC -classpath .:$OSPL_HOME/jar/dcpssaj.jar:$SPLICE_EXTRA_CP -d .. pingpong/*.java

#
# Compile application java code
#
echo "Compile application Java classes"
cd ..
$SPLICE_JAVAC -classpath .:$OSPL_HOME/jar/dcpssaj.jar:$SPLICE_EXTRA_CP -d . ../*.java 

cd ..
