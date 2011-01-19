cd ../exec

if [ "$1" = "" ]; then 

   java -classpath $OSPL_HOME/jar/dcpssaj.jar:../exec/OwnershipDataSubscriber.jar OwnershipDataSubscriber
else
   
   java -classpath $OSPL_HOME/jar/dcpssaj.jar:../exec/OwnershipDataSubscriber.jar OwnershipDataSubscriber > ../sh/$1

fi
cd ../sh
