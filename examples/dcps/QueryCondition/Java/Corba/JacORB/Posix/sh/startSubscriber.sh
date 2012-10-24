#echo === QueryConditionDataSubscriber
cd ../exec

if [ "$1" = "" ]; then 
   java -classpath $OSPL_HOME/jar/dcpssaj.jar:../exec/QueryConditionDataSubscriber.jar QueryConditionDataSubscriber MSFT&
else
   java -classpath $OSPL_HOME/jar/dcpssaj.jar:../exec/QueryConditionDataSubscriber.jar QueryConditionDataSubscriber MSFT > ../sh/$1
fi
cd ../sh