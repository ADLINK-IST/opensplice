echo === ContentFilteredTopicDataPublisher
if [ "$1" = "" ]; then java -classpath $OSPL_HOME/jar/dcpssaj.jar:../exec/ContentFilteredTopicDataPublisher.jar ContentFilteredTopicDataPublisher &
else java -classpath $OSPL_HOME/jar/dcpssaj.jar:../exec/ContentFilteredTopicDataPublisher.jar ContentFilteredTopicDataPublisher > ../exec/$1 ; fi
