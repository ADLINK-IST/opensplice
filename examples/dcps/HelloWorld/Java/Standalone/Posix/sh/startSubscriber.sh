cd ../exec

if [ "$1" = "" ]; then 

   java -classpath $OSPL_HOME/jar/dcpssaj.jar:HelloWorldDataSubscriber.jar HelloWorldDataSubscriber 
else
   
   PROGRAM="java -classpath $OSPL_HOME/jar/dcpssaj.jar:HelloWorldDataSubscriber.jar HelloWorldDataSubscriber"
   $PROGRAM > ../sh/$1
fi
cd ../sh
