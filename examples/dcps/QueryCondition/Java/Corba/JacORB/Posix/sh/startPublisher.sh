echo === ContentFilteredTopicDataPublisher
if [ "$1" = "" ]; then java -classpath $OSPL_HOME/jar/dcpssaj.jar:../exec/QueryConditionDataPublisher.jar QueryConditionDataPublisher &
else java -classpath $OSPL_HOME/jar/dcpssaj.jar:../exec/QueryConditionDataPublisher.jar QueryConditionDataPublisher > ../exec/$1 ; fi
