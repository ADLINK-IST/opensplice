# usage ./update_windows_RUN_scripts.sh examples_list language_list
# language = C  | Java | CS
if [ "$1" = "" ]; then
  echo "*** usage : ./update_windows_RUN_scripts.sh examples_list language_list"
  exit;
fi
if [ "$2" = "" ]; then
  echo "*** usage : ./update_windows_RUN_scripts.sh examples_list language_list"
  exit;
fi
LIST=$1
LANGUAGE_LIST=$2
DCPS=$PWD/../../examples/dcps
AUTOMATION_SCRIPTS=../../build/scripts/overnight/example_automation_scripts
for LANG in `cat $LANGUAGE_LIST`;do
  echo === Lang=$LANG;
  for each in `cat $LIST`;do 
    echo "    Example= $each ===";
    if [ $LANG = "C" ] || [ $LANG = "CS" ]; then
      # copy C++ SA RUN.bat to Standalone Java and C
      echo ===  copy C++ SA RUN.bat to Standalone $LANG
      echo === cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/VS2005/Bat/RUN.bat $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Standalone/VS2005/Bat/RUN.bat;
      cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/VS2005/Bat/RUN.bat $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Standalone/VS2005/Bat/RUN.bat;
      if [ $each = "Durability" ]; then
        # copy C++ SA start.bat to Standalone Java and C
        echo ===  copy C++ SA $each start.bat to Standalone  $LANG
        echo === cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/VS2005/Bat/start.bat $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Standalone/VS2005/Bat/start.bat;
        cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/VS2005/Bat/start.bat $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Standalone/VS2005/Bat/start.bat;
      fi
      # copy C++ SA start* to SA C     
      echo ===  copy C++ SA start* to SA $LANG
      echo === cp -f $DCPS/$each/C++/Standalone/VS2005/Bat/startSubscriber.bat $DCPS/$each/$LANG/Standalone/VS2005/Bat/;
      cp -f $DCPS/$each/C++/Standalone/VS2005/Bat/startSubscriber.bat $DCPS/$each/$LANG/Standalone/VS2005/Bat;
      if [ $each != "BuiltInTopics" ]; then
         echo === cp -f $DCPS/$each/C++/Standalone/VS2005/Bat/startPublisher.bat $DCPS/$each/$LANG/Standalone/VS2005/Bat/;
         cp -f $DCPS/$each/C++/Standalone/VS2005/Bat/startPublisher.bat $DCPS/$each/$LANG/Standalone/VS2005/Bat;
      fi
    fi
    # CORBA
    if [ $LANG = "C++" ]; then
      # copy C++ SA start* to Corba C++      
      echo ===  copy C++ SA start* to Corba C++ 
      echo === cp -f $DCPS/$each/C++/Standalone/VS2005/Bat/startSubscriber.bat $DCPS/$each/$LANG/Corba/OpenFusion/VS2005/Bat/;
      cp -f $DCPS/$each/C++/Standalone/VS2005/Bat/startSubscriber.bat $DCPS/$each/$LANG/Corba/OpenFusion/VS2005/Bat;
      if [ $each != "BuiltInTopics" ]; then
         echo === cp -f $DCPS/$each/C++/Standalone/VS2005/Bat/startPublisher.bat $DCPS/$each/$LANG/Corba/OpenFusion/VS2005/Bat/;
         cp -f $DCPS/$each/C++/Standalone/VS2005/Bat/startPublisher.bat $DCPS/$each/$LANG/Corba/OpenFusion/VS2005/Bat;
      fi
      # copy C++ SA RUN.bat to Corba C++
      echo ===  copy C++ SA RUN.bat to Corba C++ 
      echo === cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/VS2005/Bat/RUN.bat $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/OpenFusion/VS2005/Bat/RUN.bat;
      cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/VS2005/Bat/RUN.bat $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/OpenFusion/VS2005/Bat/RUN.bat;
      # copy C++ SA Durability start.bat to Corba C++
      if [ $each = "Durability" ]; then
        echo ===  copy C++ SA Durability start.bat to Corba C++ 
        echo === cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/VS2005/Bat/start.bat $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/OpenFusion/VS2005/Bat/start.bat;
        cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/VS2005/Bat/start.bat $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/OpenFusion/VS2005/Bat/start.bat;
      fi
    fi
    if [ $LANG = "Java" ]; then
      # copy C++ SA RUN.bat to Standalone Java and C
      echo ===  copy C++ SA RUN.bat to Standalone $LANG
      echo === cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/VS2005/Bat/RUN.bat $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Standalone/Windows/Bat/RUN.bat;
      cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/VS2005/Bat/RUN.bat $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Standalone/Windows/Bat/RUN.bat;
      if [ $each = "Durability" ]; then
        # copy C++ SA start.bat to Standalone Java and C
        echo ===  copy C++ SA $each start.bat to Standalone  $LANG
        echo === cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/VS2005/Bat/start.bat $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Standalone/Windows/Bat/start.bat;
        cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/VS2005/Bat/start.bat $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Standalone/Windows/Bat/start.bat;
      fi
      # copy C++ SA RUN.bat to Corba Java
      echo ===  copy C++ SA RUN.bat to Corba Java
      echo === cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/VS2005/Bat/RUN.bat $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/JacORB/Windows/Bat/RUN.bat;
      cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/VS2005/Bat/RUN.bat $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/JacORB/Windows/Bat/RUN.bat;
      # copy C++ SA Durability start.bat to Corba Java
       if [ $each = "Durability" ]; then
        echo ===  copy C++ SA Durability start.bat to Corba Java 
        echo === cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/VS2005/Bat/start.bat $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/JacORB/Windows/Bat/start.bat;
        cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/VS2005/Bat/start.bat $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/JacORB/Windows/Bat/start.bat;
      fi
    fi
  done
done
