if [ "$4" = "" ]; then
  ../exec/DurabilityDataPublisher $*&
else
   ../exec/DurabilityDataPublisher $1 $2 $3 >> $4&
   PID=$!
   echo "$PID"
fi


