cd ../exec

if [ "$1" = "" ]; then 

   java -classpath $OSPL_HOME/jar/dcpssaj.jar:HelloWorldDataSubscriber.jar HelloWorldDataSubscriber & 
else
   
   java -classpath $OSPL_HOME/jar/dcpssaj.jar:HelloWorldDataSubscriber.jar HelloWorldDataSubscriber > ../sh/$1
fi
cd ../sh