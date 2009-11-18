
SUM=0
SUCC=0
FAIL=0
FAILURES=0

CUR_PATH=`pwd`
echo " Begin"

for PROJECT in $EXAMPLES 
do
    echo " ### Project: $PROJECT Begin ### "
    cd "$CUR_PATH/$PROJECT"
    
    run="yes"
    
    for test in $EXCLUDED_TESTS
    do
	if [ $PROJECT = $test ]; then
	   run="no"
	fi
    done
        
    if [ $run = "yes" ]; 
    then
        sh RUN
        status=$?
        
        SUM=`expr $SUM + 1`
        if [ $status = 0 ]; then
            SUCC=`expr $SUCC + 1`
	    echo " ### Project: $PROJECT success ### "
        else
            FAIL=`expr $FAIL + 1`
	    echo " ### Project: $PROJECT failed with status: $status ### "
            FAILURES=1
        fi  
	 
        echo " ### Project: $PROJECT End ### "
        echo ""
    else
        echo "Next Examples"
    fi
done

if [ -n "`egrep -i '(segmentation|killed|timeout|file not found|NoClassDefFoundError|Assertion failed|Creation failed|failed!|fatal error)' $OSPL_HOME/../../../log/examples/run/overview.log`" ]
then
   FAILURES=1
   echo "There were failures running the examples - check log for details"
fi

cd "$CUR_PATH"

if [ $FAILURES = 1 ]; then
   exit 1
fi
