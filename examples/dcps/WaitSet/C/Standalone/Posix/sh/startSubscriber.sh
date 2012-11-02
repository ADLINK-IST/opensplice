if [ "$1" = "" ]
then 
   ../exec/WaitSetDataSubscriber
else
   ../exec/WaitSetDataSubscriber > $1 
fi
