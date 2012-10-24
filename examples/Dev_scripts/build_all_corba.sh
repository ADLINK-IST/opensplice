# usage ./build_all_corba.sh examples_list language
# language = C++ | C  | Java
if [ "$1" = "" ]; then 
  echo "*** usage : ./build_all_corba.sh examples_list language_list"
  exit;
fi
if [ "$2" = "" ]; then 
  echo "*** usage : ./build_all_corba.sh examples_list language_list"
  exit;
fi

#Checking OSPLDEV
OSPL_DEV_MOUNTED=`mount | grep kaukura:/opt/ospldev | wc -l`;
echo $OSPL_DEV_MOUNTED
if [ $OSPL_DEV_MOUNTED -lt 1 ];then
echo Please : 
echo "   - login as Root"
echo "   - enter : mkdir /opt/ospldev (if directory does not exist)"
echo "   - enter : mount -t nfs kaukura:/opt/ospldev /opt/ospldev"
exit;
fi

#. ./set_env.sh

LIST=$1
LANG_LIST=$2

if [ "$LANG" = "Java" ]; then
   echo === Setting environment variables
   . /opt/ospldev/JacORB/v2.3.0.3_V090803-15/release.com
   ORB=JacORB;
   echo "=== JACORB_HOME=$JACORB_HOME" 
else
   echo === Setting environment variables
   . /opt/ospldev/TAO/openFusion_1.6.1/release.com
   ORB=OpenFusion;
   echo "=== TAO_ROOT=$TAO_ROOT"
fi
echo "=== ORB = $ORB"

DCPS=$PWD/../../examples/dcps
SCRIPT_DIR=$PWD
echo DCPS=$DCPS
for LANG in `cat $LANG_LIST`; do 
   for each in `cat $LIST`; do 
      echo === Building $each;
      rm -f build.log
      cd  $DCPS/$each/$LANG/Corba/$ORB/Posix
      echo === Entering $PWD
      if [ "$LANG" = "Java" ]; then
        ./BUILD >> build.log 2>&1;
      else
        make -f makefile_unix>> build.log 2>&1
      fi
      NBERR=`grep Err build.log  | wc -l`
      if [ $NBERR -eq  0 ]
           then echo "=== Build OK"
      else echo "*** Build NOK"
      fi
   done
   cd $SCRIPT_DIR;
done