cd ../exec

if [ "$1" = "" ]; then 
   java -classpath $OSPL_HOME/jar/dcpssaj.jar:../exec/BuildInTopicsDataSubscriber.jar BuildInTopicsDataSubscriber
else
   java -classpath $OSPL_HOME/jar/dcpssaj.jar:../exec/BuildInTopicsDataSubscriber.jar BuildInTopicsDataSubscriber > ../sh/$1
fi
cd ../sh
