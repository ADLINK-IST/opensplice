if [ "$2" = "" ]; then 
  ../exec/DurabilityDataSubscriber $1
else
   ../exec/DurabilityDataSubscriber $1 >> $2
fi
