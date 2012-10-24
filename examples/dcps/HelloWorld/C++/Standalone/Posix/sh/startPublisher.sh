
#echo === HelloWorldPublisher

cd ../exec

if [ "$1" = "" ]; then 
   ./HelloWorldDataPublisher
else
   ./HelloWorldDataPublisher > ./$1 ; 
fi

cd ../sh
