# usage ./update_RUN_scripts_standalone.sh examples_list language
# language = C  | Java | CS
if [ "$1" = "" ]; then
  echo "*** usage : ./update_RUN_scripts_standalone.sh examples_list language"
  exit;
fi
if [ "$2" = "" ]; then
  echo "*** usage : ./update_RUN_scripts_standalone.sh examples_list language"
  exit;
fi
LIST=$1
LANG=$2
DCPS=$PWD/../../examples/dcps
for each in `cat $LIST`;do 
  echo === $each;
  echo === cp -f waiting_end_of.sh $DCPS/$each/$LANG/Standalone/Posix/sh/
  cp -f waiting_end_of.sh $DCPS/$each/$LANG/Standalone/Posix/sh/
  if [ $LANG != "C++" ]; then
    echo === cp -f $DCPS/$each/C++/Standalone/Posix/sh/RUN $DCPS/$each/$LANG/Standalone/Posix/sh/RUN;
    cp -f $DCPS/$each/C++/Standalone/Posix/sh/RUN $DCPS/$each/$LANG/Standalone/Posix/sh/RUN;
  fi
  if [ $LANG = "C" ]; then
    echo === cp -f $DCPS/$each/C++/Standalone/Posix/sh/startSubscriber.sh $DCPS/$each/$LANG/Standalone/Posix/sh/;
    cp -f $DCPS/$each/C++/Standalone/Posix/sh/startSubscriber.sh $DCPS/$each/$LANG/Standalone/Posix/sh;
    if [ $each != "BuiltInTopics" ]; then
       echo === cp -f $DCPS/$each/C++/Standalone/Posix/sh/startPublisher.sh $DCPS/$each/$LANG/Standalone/Posix/sh/;
       cp -f $DCPS/$each/C++/Standalone/Posix/sh/startPublisher.sh $DCPS/$each/$LANG/Standalone/Posix/sh;
    fi
  fi
done
if [ $LANG != "C++" ]; then
  echo === cp -f $DCPS/Durability/C++/Standalone/Posix/sh/start.sh $DCPS/Durability/$LANG/Standalone/Posix/sh/start.sh
  cp -f $DCPS/Durability/C++/Standalone/Posix/sh/start.sh $DCPS/Durability/$LANG/Standalone/Posix/sh/start.sh
fi
