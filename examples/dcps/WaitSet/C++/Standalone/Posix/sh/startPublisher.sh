echo === WaitSetDataPublisher
if [ "$1" = "" ]; then ../exec/WaitSetDataPublisher
else ../exec/WaitSetDataPublisher > ../exec/$1 ; fi
