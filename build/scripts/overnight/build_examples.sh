
SUM=0
SUCC=0
FAIL=0
FAILURES=0

CUR_PATH=`pwd`
echo " Begin"

for PROJECT in $EXAMPLES 
do
    echo " ####  Project: $PROJECT Begin####"
    cd "$CUR_PATH/$PROJECT"
    
    spec_build="no"
    
    for test in $SPECIAL_BUILD_FILES
    do
        if [ "$test" = "$PROJECT" ]; then
            spec_build="yes"
        fi
    done
    
    if [ $spec_build = "no" ]; then
     make
     status=$?
    else
     sh BUILD
     status=$?
    fi
    
    SUM=`expr $SUM + 1`
    if [ $status = 0 ]; then
       SUCC=`expr $SUCC + 1`
       echo "Build succeeded"
     else
       FAIL=`expr $FAIL + 1`
       echo "Build failed: $status"
       FAILURES=1
     fi  
    
    echo " #### Project: $PROJECT End ####" 
    echo ""
done
cd "$CUR_PATH"

if [ -n "`egrep -i '(segmentation|killed|timeout|file not found|NoClassDefFoundError|Assertion failed|Creation failed|failed!|fatal error)' $OSPL_HOME/../../../log/examples/build/summary.txt`" ]
then
   FAILURES=1
   echo "There were failures building the examples - check log for details"
fi

if [ $FAILURES = 1 ]; then
   exit 1
fi
