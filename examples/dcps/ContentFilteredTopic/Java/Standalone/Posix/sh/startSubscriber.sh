cd ../exec

echo === ContentFilteredTopicDataSubscriber
if [ "$1" = "" ]; then 
   java -classpath $OSPL_HOME/jar/dcpssaj.jar:ContentFilteredTopicDataSubscriber.jar ContentFilteredTopicDataSubscriber GE
else
   java -classpath $OSPL_HOME/jar/dcpssaj.jar:ContentFilteredTopicDataSubscriber.jar ContentFilteredTopicDataSubscriber GE  > ../sh/$1
fi
cd ../sh
