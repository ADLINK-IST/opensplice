# usage ./build_all_corba.sh examples_list language
# language = C++ | C  | Java
if [ "$2" = "" ]; then 
  echo "*** usage : ./build_all_standalone.sh examples_list language_list"
  exit;
fi
LIST=$1
LANG_LIST=$2
DCPS=$PWD/../../examples/dcps
SCRIPT_DIR=$PWD
echo DCPS=$DCPS
for LANG in `cat $LANG_LIST`; do 
   for each in `cat $LIST`; do 
      echo === Building $each;
      rm -f build.log
      cd  $DCPS/$each/$LANG/Standalone/Posix
      echo === Entering $DCPS/$each/$LANG/Standalone/Posix
      if [ "$LANG" == "Java" ]; then
        ./BUILD >> build.log 2>&1;
      else
        make -f makefile_unix>> build.log 2>&1;
      fi
      NBERR=`grep -i Err build.log  | wc -l`
      if [ $NBERR -eq  0 ]
           then echo "=== Build OK"
      else echo "*** Build NOK"
      fi
   done
   cd $SCRIPT_DIR;
done