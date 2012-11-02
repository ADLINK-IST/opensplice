echo === ListenerDataPublisher
if [ "$1" = "" ]; then ../exec/ListenerDataPublisher
else ../exec/ListenerDataPublisher > ../exec/$1 ; fi
