# usage ./update_myRUN_scripts.sh examples_list language_list
# language = C  | Java | CS
if [ "$1" = "" ]; then
  echo "*** usage : ./update_myRUN_scripts.sh examples_list language_list"
  exit;
fi
if [ "$2" = "" ]; then
  echo "*** usage : ./update_myRUN_scripts.sh examples_list language_list"
  exit;
fi
LIST=$1
LANGUAGE_LIST=$2
DCPS=$PWD/../../examples/dcps
AUTOMATION_SCRIPTS=$PWD/../../build/scripts/overnight/example_automation_scripts
for LANG in `cat $LANGUAGE_LIST`;do
  for each in `cat $LIST`;do 
    echo === $each;
    ln -s $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Standalone/Posix/sh/RUN $DCPS/$each/$LANG/Standalone/Posix/sh/RUN
    if [ $LANG = "C++" ]; then
      ln -s $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/OpenFusion/Posix/sh/RUN $DCPS/$each/C++/Corba/OpenFusion/Posix/sh/RUN;
      if [ $each = "Durability" ]; then
         ln -s $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/Posix/sh/start.sh $DCPS/$each/C++/Corba/OpenFusion/Posix/sh/start.sh;
      fi
    fi
    if [ $LANG = "Java" ]; then
      ln -s $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/JacORB/Posix/sh/RUN $DCPS/$each/$LANG/Corba/JacORB/Posix/sh/RUN;
      if [ $each = "Durability" ]; then
        ln -s $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/JacORB/Posix/sh/start.sh $DCPS/$each/$LANG/Corba/JacORB/Posix/sh/start.sh;
      fi
   fi
  done
done
