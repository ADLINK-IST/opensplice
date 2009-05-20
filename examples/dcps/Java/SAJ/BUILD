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
javac -classpath .:$OSPL_HOME/jar/dcpssaj.jar -d .. pingpong/*.java

#
# Compile application java code
#
echo "Compile application Java classes"
cd ..
javac -classpath .:$OSPL_HOME/jar/dcpssaj.jar -d . ../*.java 

cd ..
