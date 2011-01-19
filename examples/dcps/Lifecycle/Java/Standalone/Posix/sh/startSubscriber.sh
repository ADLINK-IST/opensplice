#echo === LifecycleDataSubscriber
cd ../exec

if [ "$1" = "" ]; then 
   java -classpath $OSPL_HOME/jar/dcpssaj.jar:LifecycleDataSubscriber.jar LifecycleDataSubscriber
else
   PROGRAM="java -classpath $OSPL_HOME/jar/dcpssaj.jar:LifecycleDataSubscriber.jar LifecycleDataSubscriber "
   $PROGRAM > ../sh/$1 
fi
cd ../sh
