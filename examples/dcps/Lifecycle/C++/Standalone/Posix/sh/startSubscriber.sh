if [ "$1" = "" ]; then 
   ../exec/LifecycleDataSubscriber
else
   ../exec/LifecycleDataSubscriber > $1
fi
