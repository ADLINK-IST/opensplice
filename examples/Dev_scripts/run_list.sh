# usage ./run_all_standalone.sh examples_list language
# language = C++ | C  | Java
if [ "$1" = "" ]; then 
  echo "*** usage : ./run_all_standalone.sh examples_list language_list"
  exit;
fi
if [ "$2" = "" ]; then 
  echo "*** usage : ./run_all_standalone.sh examples_list language_list"
  exit;
fi
LIST=$1
LANG_LIST=$2
DCPS=$PWD/../../examples/dcps
SCRIPT_DIR=$PWD
ospl stop
sleep 2
for LANG in `cat $LANG_LIST`; do
  for each in `cat $LIST`; do
     echo ============================================
     echo === Running $each for $LANG Standalone;
     echo ============================================
     cd $DCPS/$each/$LANG/Standalone/Posix/sh
     echo === Entering $DCPS/$each/$LANG/Standalone/Posix/sh
     ./RUN
     if [ $LANG = "C++" ]; then
       echo ============================================
       echo === Running $each for $LANG Corba/OpenFusion;
       echo ============================================
       cd $DCPS/$each/$LANG/Corba/OpenFusion/Posix/sh
       echo === Entering $DCPS/$each/$LANG/Corba/OpenFusion/Posix/sh
       ./RUN
     fi
     if [ $LANG = "Java" ]; then
       echo ============================================
       echo === Running $each for $LANG Corba/JacORB;
       echo ============================================
       cd $DCPS/$each/$LANG/Corba/JacORB/Posix/sh
       echo === Entering $DCPS/$each/$LANG/Corba/JacORB/Posix/sh
       ./RUN
     fi
  done
  cd $SCRIPT_DIR
done
