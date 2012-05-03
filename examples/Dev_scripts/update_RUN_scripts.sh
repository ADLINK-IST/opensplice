# usage ./update_RUN_scripts.sh examples_list language_list
# language = C  | Java | CS
if [ "$1" = "" ]; then
  echo "*** usage : ./update_RUN_scripts.sh examples_list language_list"
  exit;
fi
if [ "$2" = "" ]; then
  echo "*** usage : ./update_RUN_scripts.sh examples_list language_list"
  exit;
fi
LIST=$1
LANGUAGE_LIST=$2
DCPS=$PWD/../../examples/dcps
AUTOMATION_SCRIPTS=../../build/scripts/overnight/example_automation_scripts
for LANG in `cat $LANGUAGE_LIST`;do
  for each in `cat $LIST`;do 
    echo === $each;
    if [ $LANG != "C++" ]; then
      # copy C++ SA RUN to Standalone Java and C
      echo ===  copy C++ SA RUN to Standalone $LANG
      echo === cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/Posix/sh/RUN $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Standalone/Posix/sh/RUN;
      cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/Posix/sh/RUN $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Standalone/Posix/sh/RUN;
      if [ $each = "Durability" ]; then
        # copy C++ SA start.sh to Standalone Java and C
        echo ===  copy C++ SA $each start.sh to Standalone  $LANG
        echo === cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/Posix/sh/start.sh $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Standalone/Posix/sh/start.sh;
        cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/Posix/sh/start.sh $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Standalone/Posix/sh/start.sh;
      fi
    fi
    if [ $LANG = "C" ]; then
      # copy C++ SA start* to SA C     
      echo ===  copy C++ SA start* to SA C
      echo === cp -f $DCPS/$each/C++/Standalone/Posix/sh/startSubscriber.sh $DCPS/$each/$LANG/Standalone/Posix/sh/;
      cp -f $DCPS/$each/C++/Standalone/Posix/sh/startSubscriber.sh $DCPS/$each/$LANG/Standalone/Posix/sh;
      if [ $each != "BuiltInTopics" ]; then
         echo === cp -f $DCPS/$each/C++/Standalone/Posix/sh/startPublisher.sh $DCPS/$each/$LANG/Standalone/Posix/sh/;
         cp -f $DCPS/$each/C++/Standalone/Posix/sh/startPublisher.sh $DCPS/$each/$LANG/Standalone/Posix/sh;
      fi
    fi
    # CORBA
    if [ $LANG = "C++" ]; then
      # copy C++ SA start* to Corba C++      
      echo ===  copy C++ SA start* to Corba C++ 
      echo === cp -f $DCPS/$each/C++/Standalone/Posix/sh/startSubscriber.sh $DCPS/$each/$LANG/Corba/OpenFusion/Posix/sh/;
      cp -f $DCPS/$each/C++/Standalone/Posix/sh/startSubscriber.sh $DCPS/$each/$LANG/Corba/OpenFusion/Posix/sh;
      if [ $each != "BuiltInTopics" ]; then
         echo === cp -f $DCPS/$each/C++/Standalone/Posix/sh/startPublisher.sh $DCPS/$each/$LANG/Corba/OpenFusion/Posix/sh/;
         cp -f $DCPS/$each/C++/Standalone/Posix/sh/startPublisher.sh $DCPS/$each/$LANG/Corba/OpenFusion/Posix/sh;
      fi
      # copy C++ SA RUN to Corba C++
      echo ===  copy C++ SA RUN to Corba C++ 
      echo === cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/Posix/sh/RUN $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/OpenFusion/Posix/sh/RUN;
      cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/Posix/sh/RUN $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/OpenFusion/Posix/sh/RUN;
      # copy C++ SA Durability start.sh to Corba C++
      if [ $each = "Durability" ]; then
        echo ===  copy C++ SA Durability start.sh to Corba C++ 
        echo === cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/Posix/sh/start.sh $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/OpenFusion/Posix/sh/start.sh;
        cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/Posix/sh/start.sh $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/OpenFusion/Posix/sh/start.sh;
      fi
    fi
    if [ $LANG = "Java" ]; then
      # copy C++ SA RUN to Corba Java
      echo ===  copy C++ SA RUN to Corba Java
      echo === cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/Posix/sh/RUN $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/JacORB/Posix/sh/RUN;
      cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/Posix/sh/RUN $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/JacORB/Posix/sh/RUN;
      # copy C++ SA Durability start.sh to Corba Java
       if [ $each = "Durability" ]; then
        echo ===  copy C++ SA Durability start.sh to Corba Java 
        echo === cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/Posix/sh/start.sh $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/JacORB/Posix/sh/start.sh;
        cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/Posix/sh/start.sh $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/JacORB/Posix/sh/start.sh;
      fi
    fi

  done
done
