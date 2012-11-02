#echo === HelloWorldPublisher

cd ../exec

if [ "$1" = "" ]; then 
	  java -classpath $OSPL_HOME/jar/dcpssaj.jar:HelloWorldDataPublisher.jar HelloWorldDataPublisher &
   else
	      java -classpath $OSPL_HOME/jar/dcpssaj.jar:HelloWorldDataPublisher.jar HelloWorldDataPublisher & > ./$1 ; 
      fi

cd ../sh
