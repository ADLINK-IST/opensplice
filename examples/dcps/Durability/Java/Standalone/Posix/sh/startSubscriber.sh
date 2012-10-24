cd ../exec

if [ "$2" = "" ]; then 
   java -classpath $OSPL_HOME/jar/dcpssaj.jar:DurabilityDataSubscriber.jar DurabilityDataSubscriber $1
else
   java -classpath $OSPL_HOME/jar/dcpssaj.jar:DurabilityDataSubscriber.jar DurabilityDataSubscriber $1 >> ../sh/$2
fi
cd ../sh
