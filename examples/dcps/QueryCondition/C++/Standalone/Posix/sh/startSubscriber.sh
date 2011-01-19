if [ "$1" = "" ]
then 
   ../exec/QueryConditionDataSubscriber MSFT
else
   ../exec/QueryConditionDataSubscriber MSFT > $1
fi
