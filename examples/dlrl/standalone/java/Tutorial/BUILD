mkdir -p bld/idlpp

#
# Generate java classes from DCPS IDL
#
echo "Compiling DCPS IDL with SPLICE IDL compiler"
idlpp -S -l java -d bld/idlpp data_files/ExtChat.idl

#
# Compile generated java code
#
echo "Compiling generated Java classes"
javac -classpath $OSPL_HOME/jar/dcpssaj.jar -d bld -sourcepath bld/idlpp bld/idlpp/Chat/*.java

#
# Generate java classes from DLRL IDL and XML mapping file for WhiteListedMessageboard
#
echo "Compiling DLRL code for WhiteListedMessageBoard application"
mkdir -p bld/ospldcg/WhiteListedMessageBoard

ospldcg -dcps data_files/ExtChat.idl -dlrl data_files/WhiteListObjects.idl -mapping data_files/mapping.xml -l JAVA -o bld/ospldcg/WhiteListedMessageBoard

#
# Generate java classes from DLRL IDL and XML mapping file for WhiteListEditor and WhiteListViewer
#
echo "Compiling DLRL code for WhiteListEditor and WhiteListViewer applications"
mkdir -p bld/ospldcg/WhiteListEditor
ospldcg -dcps data_files/ExtChat.idl -dlrl data_files/WhiteListObjects_editor.idl -mapping data_files/mapping_editor.xml -l JAVA -o bld/ospldcg/WhiteListEditor

#
# Compile generated java code
#
mkdir -p bld/WhiteListedMessageBoard
mkdir -p bld/WhiteListEditor
echo "Compiling generated Java classes"
javac -classpath bld:$OSPL_HOME/jar/dcpssaj.jar:$OSPL_HOME/jar/dlrlsaj.jar -d bld/WhiteListedMessageBoard -sourcepath bld/ospldcg/WhiteListedMessageBoard bld/ospldcg/WhiteListedMessageBoard/DLRLChat/*.java
javac -classpath bld:$OSPL_HOME/jar/dcpssaj.jar:$OSPL_HOME/jar/dlrlsaj.jar -d bld/WhiteListEditor -sourcepath bld/ospldcg/WhiteListEditor bld/ospldcg/WhiteListEditor/DLRLChat/*.java

#
# Compile application java code
#
echo "Compiling application Java classes"
javac -classpath $OSPL_HOME/jar/dcpssaj.jar:$OSPL_HOME/jar/dlrlsaj.jar:bld -d bld Common/*.java
javac -classpath $OSPL_HOME/jar/dcpssaj.jar:$OSPL_HOME/jar/dlrlsaj.jar:bld:bld/WhiteListedMessageBoard -d bld WhiteListedMessageBoard/*.java
javac -classpath $OSPL_HOME/jar/dcpssaj.jar:$OSPL_HOME/jar/dlrlsaj.jar:bld:bld/WhiteListEditor -d bld WhiteListViewer/*.java
javac -classpath $OSPL_HOME/jar/dcpssaj.jar:$OSPL_HOME/jar/dlrlsaj.jar:bld:bld/WhiteListEditor -d bld WhiteListEditor/*.java
