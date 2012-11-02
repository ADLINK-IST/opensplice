mkdir -p exec
mkdir -p bld
cd bld
#
# Compile application java code
#
echo "Compile application Java classes"

pwd
$SPLICE_JAVAC -classpath .:$OSPL_HOME/jar/dcpssaj.jar:$SPLICE_EXTRA_CP -d ./ ../../../src/*.java
jar cf ../exec/BuildInTopicsDataSubscriber.jar BuildInTopicsDataSubscriber.class
cd ..

