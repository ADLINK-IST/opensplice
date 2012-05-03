# usage ./update_windows_myRUN.bat_scripts.sh examples_list language_list
# language = C  | Java | CS
if [ "$1" = "" ]; then
  echo "*** usage : ./update_windows_myRUN.bat_scripts.sh examples_list language_list"
  exit;
fi
if [ "$2" = "" ]; then
  echo "*** usage : ./update_windows_myRUN.bat_scripts.sh examples_list language_list"
  exit;
fi
LIST=$1
LANGUAGE_LIST=$2
DCPS=$PWD/../../examples/dcps
AUTOMATION_SCRIPTS=$PWD/../../build/scripts/overnight/example_automation_scripts
for LANG in `cat $LANGUAGE_LIST`;do
  echo === Lang=$LANG;
  for each in `cat $LIST`;do  
    echo "    Example= $each ===";
    if [ $LANG = "C" ] || [ $LANG = "CS" ]; then
       cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Standalone/VS2005/Bat/RUN.bat $DCPS/$each/$LANG/Standalone/VS2005/Bat/RUN.bat
	   cp -f myRUN.bat $DCPS/$each/$LANG/Standalone/VS2005/Bat/myRUN.bat
	fi
    if [ $LANG = "C++" ]; then
       cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Standalone/VS2005/Bat/RUN.bat $DCPS/$each/$LANG/Standalone/VS2005/Bat/RUN.bat
       cp -f myRUN.bat $DCPS/$each/$LANG/Standalone/VS2005/Bat/myRUN.bat
      cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/OpenFusion/VS2005/Bat/RUN.bat $DCPS/$each/C++/Corba/OpenFusion/VS2005/Bat/RUN.bat;
	  cp -f myRUN.bat $DCPS/$each/C++/Corba/OpenFusion/VS2005/Bat/myRUN.bat;
      if [ $each = "Durability" ]; then
         cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/C++/Standalone/VS2005/Bat/start.bat $DCPS/$each/C++/Corba/OpenFusion/VS2005/Bat/start.bat;
      fi
    fi
    if [ $LANG = "Java" ]; then
      cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Standalone/Windows/Bat/RUN.bat $DCPS/$each/$LANG/Standalone/Windows/Bat/RUN.bat
      cp -f myRUN.bat $DCPS/$each/$LANG/Standalone/Windows/Bat/myRUN.bat
      cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/JacORB/Windows/Bat/RUN.bat $DCPS/$each/$LANG/Corba/JacORB/Windows/Bat/RUN.bat;
      cp -f myRUN.bat $DCPS/$each/$LANG/Corba/JacORB/Windows/Bat/myRUN.bat;
      if [ $each = "Durability" ]; then
        cp -f $AUTOMATION_SCRIPTS/examples/dcps/$each/$LANG/Corba/JacORB/Windows/Bat/start.bat $DCPS/$each/$LANG/Corba/JacORB/Windows/Bat/start.bat;
      fi
   fi
  done
done
