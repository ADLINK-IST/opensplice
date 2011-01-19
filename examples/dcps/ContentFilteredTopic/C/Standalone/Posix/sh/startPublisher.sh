echo === ContentFilteredTopicDataPublisher
if [ "$1" = "" ]; then ../exec/ContentFilteredTopicDataPublisher
else ../exec/ContentFilteredTopicDataPublisher > ../exec/$1 ; fi
