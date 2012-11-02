#echo === WaitSetDataSubscriber
cd ../exec

if [ "$1" = "" ]; then 

   java -classpath $OSPL_HOME/jar/dcpssaj.jar:WaitSetDataSubscriber.jar WaitSetDataSubscriber & 
else
   
   java -classpath $OSPL_HOME/jar/dcpssaj.jar:WaitSetDataSubscriber.jar WaitSetDataSubscriber > ../sh/$1
fi
cd ../sh