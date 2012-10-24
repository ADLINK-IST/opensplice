echo === ContentFilteredTopicDataPublisher
if [ "$1" = "" ]; then ../exec/QueryConditionDataPublisher
else ../exec/QueryConditionDataPublisher > ../exec/$1 ; fi
