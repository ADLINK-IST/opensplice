if [ "$2" = "" ]; then 
  ../exec/DurabilityDataSubscriber $1
else
   PROGRAM="../exec/DurabilityDataSubscriber"
   $PROGRAM  $1 >> $2 &
   PID=$!
   echo "$PID"
fi
