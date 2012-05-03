if [ "$1" = "" ]
then 
   ../exec/ListenerDataSubscriber
else
   ../exec/ListenerDataSubscriber > $1
fi
