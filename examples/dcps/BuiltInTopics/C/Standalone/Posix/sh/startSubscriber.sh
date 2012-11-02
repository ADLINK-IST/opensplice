if [ "$1" = "" ]; 
then 
   ../exec/BuiltInTopicsDataSubscriber
else
   ../exec/BuiltInTopicsDataSubscriber > $1
fi
