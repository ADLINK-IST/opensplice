# usage ./run_all_standalone.sh examples_list language
# language = C++ | C  | Java
if [ "$1" = "" ]; then 
  echo "*** usage : ./run_all_standalone.sh examples_list language"
  exit;
fi
if [ "$2" = "" ]; then 
  echo "*** usage : ./run_all_standalone.sh examples_list language"
  exit;
fi
LIST=$1
LANG=$2
DCPS=$PWD/../../examples/dcps
ospl stop
sleep 2
for each in `cat $LIST`; do
   echo ============================================
   echo === Running $each;
   echo ============================================
   cd $DCPS/$each/$LANG/Standalone/Posix/sh
   echo === Entering $DCPS/$each/C++/Standalone/Posix/sh
   ./RUN
done