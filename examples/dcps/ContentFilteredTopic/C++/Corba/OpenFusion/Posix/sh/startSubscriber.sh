echo === ContentFilteredTopicDataSubscriber
if [ "$1" = "" ] 
then 
   ../exec/ContentFilteredTopicDataSubscriber GE
else
   ../exec/ContentFilteredTopicDataSubscriber GE > $1 
fi
