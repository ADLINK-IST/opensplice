if [ "$1" = "" ]
then 
   ../exec/OwnershipDataSubscriber
else
   ../exec/OwnershipDataSubscriber > $1
fi
