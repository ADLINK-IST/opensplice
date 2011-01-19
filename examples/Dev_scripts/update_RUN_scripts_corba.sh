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
if [ "$LANG" = "Java" ]; then
ORB=JacORB
else
ORB=OpenFusion
fi

DCPS=$PWD/../../examples/dcps
for each in `cat $LIST`;do 
  echo === $each;
  echo === cp -f waiting_end_of.sh $DCPS/$each/$LANG/Corba/$ORB/Posix/sh/
  cp -f waiting_end_of.sh $DCPS/$each/$LANG/Corba/$ORB/Posix/sh/
  echo === cp -f $DCPS/$each/C++/Standalone/Posix/sh/RUN $DCPS/$each/$LANG/Corba/$ORB/Posix/sh/RUN;
  cp -f $DCPS/$each/C++/Standalone/Posix/sh/RUN $DCPS/$each/$LANG/Corba/$ORB/Posix/sh/RUN;
  if [ "$LANG" = "C++" ];then
     echo === cp -f $DCPS/$each/C++/Standalone/Posix/sh/start* $DCPS/$each/$LANG/Corba/$ORB/Posix/sh/;
     cp -f $DCPS/$each/C++/Standalone/Posix/sh/start* $DCPS/$each/$LANG/Corba/$ORB/Posix/sh/;
  fi
done
echo === cp -f $DCPS/Durability/C++/Standalone/Posix/sh/start.sh $DCPS/$each/$LANG/Corba/$ORB/Posix/sh/start.sh
cp -f $DCPS/Durability/C++/Standalone/Posix/sh/start.sh $DCPS/$each/$LANG/Corba/$ORB/Posix/sh/start.sh

