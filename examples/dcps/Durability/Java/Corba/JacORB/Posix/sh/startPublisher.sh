cd ../exec
if [ "$4" = "" ]; then
   java -classpath $OSPL_HOME/jar/dcpssaj.jar:DurabilityDataPublisher.jar DurabilityDataPublisher $*&
else
   java -classpath $OSPL_HOME/jar/dcpssaj.jar:DurabilityDataPublisher.jar DurabilityDataPublisher $1 $2 $3 >> $4&
   PID=$!
   echo "$PID"
fi

cd ../sh

