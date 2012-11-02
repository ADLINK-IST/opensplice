if [ "$1" = "" ]; then 
   ../exec/HelloWorldDataSubscriber
else
   ../exec/HelloWorldDataSubscriber > $1
fi
