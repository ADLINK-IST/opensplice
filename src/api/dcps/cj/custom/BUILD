. ./orbdeps

mkdir -p bld/$OSPL_TARGET

echo "Compiling DDS IDL..."
$JAVA_ORB_IDL_COMPILER $GEN_OUT_DIR_FLAG code $EXTRA_FLAGS $OSPL_HOME/etc/idl/dds_builtinTopics.idl
idlpp -l java -C -d code $OSPL_HOME/etc/idl/dds_builtinTopics.idl
echo "IDL Compilation complete."

echo "Compiling generated java code for library..."
$SPLICE_JAVAC -classpath "bld/$OSPL_TARGET:$SPLICE_EXTRA_CP"  -source 1.5 -target 1.5 -sourcepath 'code' -d bld/$OSPL_TARGET -g:none -nowarn code/DDS/*.java
$SPLICE_JAVAC -classpath "bld/$OSPL_TARGET:$SPLICE_EXTRA_CP"  -source 1.5 -target 1.5 -sourcepath 'code' -d bld/$OSPL_TARGET -g:none -nowarn code/org/opensplice/dds/dcps/*.java
echo "Java code compilation complete."

echo "Generating jar file from compiled classes"
jar cf dcpscj.jar -C bld/$OSPL_TARGET .
echo "Jar file generation complete."
