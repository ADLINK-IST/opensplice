echo === WaitSetDataPublisher
if [ "$1" = "" ]; then java -classpath $OSPL_HOME/jar/dcpssaj.jar:../exec/WaitSetDataPublisher.jar WaitSetDataPublisher &
else java -classpath $OSPL_HOME/jar/dcpssaj.jar:../exec/WaitSetDataPublisher.jar WaitSetDataPublisher & > ../exec/$1 ; fi
