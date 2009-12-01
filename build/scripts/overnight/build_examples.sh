
SUM=0
SUCC=0
FAIL=0
BUILD_RESULTS_LOG=$LOGDIR/examples/build/build_summary.txt
BUILD_LOG=$LOGDIR/examples/build/build_results.txt
SUMMARY_LOG=$LOGDIR/examples/build/build_summary.log

CUR_PATH=`pwd`

echo " Begin building examples -  `date`"

for PROJECT in $EXAMPLES 
do
    cd "$CUR_PATH/$PROJECT"
    
    spec_build="no"
    
    for test in $SPECIAL_BUILD_FILES
    do
        if [ "$test" = "$PROJECT" ]; then
            spec_build="yes"
        fi
    done

    echo " ####  Project: $PROJECT Begin####" > build.log 

    if [ $spec_build = "no" ]; then
     make >> build.log 2>&1
     status=$?
    else
     sh BUILD >> build.log 2>&1
     status=$?
    fi
    
    SUM=`expr $SUM + 1`
    if [ $status = 0 ]; 
    then
        if [ -n "`egrep -i '(segmentation|killed|timeout|file not found|NoClassDefFoundError|error|cannot find symbol)' $CUR_PATH/$PROJECT/build.log`" ]
        then
           FAIL=`expr $FAIL + 1`

           SEGMENTATIONFAULTS=`grep -ci "segmentation" $CUR_PATH/$PROJECT/build.log`
           FILENOTFOUND=`grep -ci "file not found" $CUR_PATH/$PROJECT/build.log`
           NOCLASSDEFFOUND=`grep -ci "NoClassDefFoundError" $CUR_PATH/$PROJECT/build.log`
           ERROR=`grep -ci "error" $CUR_PATH/$PROJECT/build.log`
           CANNOTFINDSYMBOL=`grep -ci "cannot find symbol" $CUR_PATH/$PROJECT/build.log`
           KILLED=`grep -ci "killed" $CUR_PATH/$PROJECT/build.log`

           echo "$PROJECT  BUILD FAILED " >> build.log
           echo "$PROJECT BUILD FAILED " >> $BUILD_RESULTS_LOG
           echo "" >> $BUILD_RESULTS_LOG
           echo "Segmentation Faults       = $SEGMENTATIONFAULTS" >> $BUILD_RESULTS_LOG
           echo "File not found errors     = $FILENOTFOUND" >> $BUILD_RESULTS_LOG
           echo "NoClassDefFound errors    = $NOCLASSDEFFOUND" >> $BUILD_RESULTS_LOG
           echo "Errors                    = $ERROR" >> $BUILD_RESULTS_LOG
           echo "Cannot find symbol errors = $CANNOTFINDSYMBOL" >> $BUILD_RESULTS_LOG
           echo "Killed                    = $KILLED" >> $BUILD_RESULTS_LOG
           echo ""  >> $BUILD_RESULTS_LOG
           echo "See build_results.txt for full details of failures"  >> $BUILD_RESULTS_LOG
           echo ""  >> $BUILD_RESULTS_LOG         
        else
           SUCC=`expr $SUCC + 1`
           echo "$PROJECT BUILD PASSED" >> $BUILD_RESULTS_LOG
           echo "$PROJECT BUILD PASSED" >> build.log 
        fi
     else
       FAIL=`expr $FAIL + 1`
       echo "$PROJECT BUILD FAILED " >> build.log
       echo "$PROJECT BUILD FAILED : $status" >> $BUILD_RESULTS_LOG
     fi  
    
    echo " #### Project: $PROJECT End ####" >> build.log 
    echo "" >> build.log

    # Add the logging for this example to a file that will contain output from all examples
    cat build.log >> $BUILD_LOG
    sleep 10

done
cd "$CUR_PATH"

# Add the summary to the start of the file

echo "" >> $SUMMARY_LOG
echo "############# Summary of Build ##########" >> $SUMMARY_LOG
echo "     Examples Built   : $SUM" >> $SUMMARY_LOG
echo "     Builds passed    : $SUCC" >> $SUMMARY_LOG 
echo "     Builds failed    : $FAIL" >> $SUMMARY_LOG 
echo "#########################################" >> $SUMMARY_LOG
echo "" >> $SUMMARY_LOG

echo "RESULTS ...." >> $SUMMARY_LOG
echo ""
cat $BUILD_RESULTS_LOG >> $SUMMARY_LOG
sleep 10

rm $BUILD_RESULTS_LOG
sleep 5

if [ $FAIL != 0 ]; then

    exit 1
fi
