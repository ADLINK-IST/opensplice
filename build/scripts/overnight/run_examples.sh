
SUM=0
SUCC=0
FAIL=0

function check_platform()
{
   CURRENT_PL_LINUX=`uname | grep Linux`
   CURRENT_PL_CYGWIN=`uname | grep CYGWIN`

  if [ "$CURRENT_PL_LINUX" != "" ] ;then
    return 0
  elif [ "$CURRENT_PL_CYGWIN" != "" ];then
    return 1
  else 
   echo UNKNOWN Platform:
   echo Current platform: `uname`
   exit
  fi
}

CUR_PATH=`pwd`
echo " Begin"

for PROJECT in $EXAMPLES 
do
    echo " ### Project: $PROJECT Begin ### "
    cd "$CUR_PATH/$PROJECT"
    
    run="yes"
    
    for test in $EXCLUDED_TESTS
    do
	if [ $PROJECT == $test ]; then
	   run="no"
	fi
    done
        
    if [ $run == "yes" ]; then
        if check_platform ; then
            sh RUN
            status=$?
        else
	    ./RUN.bat
            status=$?
	fi
        
        SUM=`expr $SUM + 1`
        if [ $status = 0 ]; then
            SUCC=`expr $SUCC + 1`
	    echo " ### Project: $PROJECT success ### "
        else
            FAIL=`expr $FAIL + 1`
	    echo " ### Project: $PROJECT failed with status: $status ### "
        fi  
	 
     echo " ### Project: $PROJECT End ### "
     echo ""
    else
     echo "Next Examples"
    fi
done

echo "############ Summary of Run:  #############"
echo "    Summary: $SUM"
echo "    Successed: $SUCC"
echo "    Failed: $FAIL"
echo "#########################"

cd "$CUR_PATH"

