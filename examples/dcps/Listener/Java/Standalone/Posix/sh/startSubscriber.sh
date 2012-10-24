#echo === ListenerDataSubscriber
cd ../exec

if [ "$1" = "" ]; then 
   java -classpath $OSPL_HOME/jar/dcpssaj.jar:ListenerDataSubscriber.jar ListenerDataSubscriber
else
   PROGRAM="java -classpath $OSPL_HOME/jar/dcpssaj.jar:ListenerDataSubscriber.jar ListenerDataSubscriber "
   $PROGRAM > ../sh/$1 
fi
cd ../sh








