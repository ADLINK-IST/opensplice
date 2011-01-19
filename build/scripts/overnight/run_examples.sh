
SUM=0
SUCC=0
FAIL=0
RUN_SUMMARY_LOG=$LOGDIR/examples/run/run_results_summary.txt
RUN_LOG=$LOGDIR/examples/run/run_results.txt
SUMMARY_LOG=$LOGDIR/examples/run/examples.log

CUR_PATH=`pwd`
echo " Begin running examples - `date`"

for PROJECT in $EXAMPLES 
do
    if [ -f $CUR_PATH/$PROJECT/RUN ]
    then
        run="yes"
        cd "$CUR_PATH/$PROJECT"

        for test in $EXCLUDED_TESTS
        do
	    if [ $PROJECT = $test ]
            then
	        run="no"
	    fi
        done

        echo " ### Project: $PROJECT Begin ### " > run.log
        
        if [ $run = "yes" ]; 
        then
            sh RUN >> run.log 2>&1
            status=$?

            SUM=`expr $SUM + 1`
            if [ $status = 0 ]
            then
               # ignore the print out from ospl describing the location of the error log
               if [ -n "`egrep -i '(segmentation|killed|timeout|file not found|No such file or directory|NoClassDefFoundError|Assertion failed|Creation of kernel failed|error)' $CUR_PATH/$PROJECT/run.log | grep -v '^Error log :'`" ]
               then               
                   FAIL=`expr $FAIL + 1`

                   ERROR=`grep -ci "error" $CUR_PATH/$PROJECT/run.log`

                   SEGMENTATIONFAULTS=`grep -ci "segmentation" $CUR_PATH/$PROJECT/run.log`
                   FILENOTFOUND=`grep -ci "file not found" $CUR_PATH/$PROJECT/run.log`
                   ASSERTIONFAILED=`grep -ci "assertion failed" $CUR_PATH/$PROJECT/run.log`
                   NOCLASSDEFFOUND=`grep -ci "NoClassDefFoundError" $CUR_PATH/$PROJECT/run.log`
                   CREATIONOFKERNEL=`grep -ci "creation of kernel failed" $CUR_PATH/$PROJECT/run.log`
                   TIMEOUTS=`grep -ci "timeout" $CUR_PATH/$PROJECT/run.log`
                   KILLED=`grep -ci "killed" $CUR_PATH/$PROJECT/run.log`

                   echo "Run $PROJECT FAILED " >> run.log
                   echo "Run $PROJECT FAILED " >> $RUN_SUMMARY_LOG
                   echo "" >> $RUN_SUMMARY_LOG
                   echo "Segmentation Faults         = $SEGMENTATIONFAULTS" >> $RUN_SUMMARY_LOG
                   echo "File not found errors       = $FILENOTFOUND" >> $RUN_SUMMARY_LOG
                   echo "NoClassDefFound errors      = $NOCLASSDEFFOUND" >> $RUN_SUMMARY_LOG
                   echo "Errors                      = $ERROR" >> $RUN_SUMMARY_LOG
                   echo "Creation of kernel failures = $CREATIONOFKERNEL" >> $RUN_SUMMARY_LOG
                   echo "Timeouts                    = $TIMEOUTS" >> $RUN_SUMMARY_LOG
                   echo "Killed                      = $KILLED" >> $RUN_SUMMARY_LOG
                   echo "Assertion Failed            = $ASSERTIONFAILED" >> $RUN_SUMMARY_LOG
                   echo "" >> $RUN_SUMMARY_LOG
                   echo "See run_results.txt for full details of failures " >> $RUN_SUMMARY_LOG           
                   echo "" >> $RUN_SUMMARY_LOG
                elif [ -f $CUR_PATH/$PROJECT/ospl-error.log ]
                then
                    FAIL=`expr $FAIL + 1` 
                    echo "" >> $RUN_SUMMARY_LOG
                    echo "Run $PROJECT FAILED ospl-error.log found " >> run.log                  
                    echo "Run $PROJECT FAILED ospl-error.log found " >> $RUN_SUMMARY_LOG     
                    echo "" >> $RUN_SUMMARY_LOG      
            elif [ -n "`egrep 'WARNING' $CUR_PATH/$PROJECT/ospl-info.log`" ]
                then
                    FAIL=`expr $FAIL + 1` 
                    echo "" >> $RUN_SUMMARY_LOG
                    echo "Run $PROJECT FAILED - WARNING found in ospl-info.log" >> run.log                  
                    echo "Run $PROJECT FAILED - WARNING found in ospl-info.log" >> $RUN_SUMMARY_LOG     
                    echo "" >> $RUN_SUMMARY_LOG       
                else
                    SUCC=`expr $SUCC + 1`
	            echo " ### Run $PROJECT PASSED ### " >> run.log
	            echo " ### Run $PROJECT PASSED ### " >> $RUN_SUMMARY_LOG
                fi
            else
                FAIL=`expr $FAIL + 1`
	            echo " ### Run $PROJECT FAILED with status: $status ### " >> run.log
	            echo " ### Run $PROJECT FAILED with status: $status ### " >> $RUN_SUMMARY_LOG
            fi  

            echo " ### Project: $PROJECT End ### " >> run.log
            echo "" >> run.log

            # Copy the logs from the example run to the log directory so they are stored on the scoreboard
            # This will include the run.log and any ospl info or error logs
            EXAMPLELOGS=`echo $PROJECT | sed -e 's/standalone/SA/' -e 's/CORBA/C/' -e 's/Java/J/' -e 's/C++/CPP/' -e 's/JacORB//' -e 's/OpenFusion//' -e 's/\///g'`

            mkdir $LOGDIR/examples/run/$EXAMPLELOGS
            cp $CUR_PATH/$PROJECT/*.log $LOGDIR/examples/run/$EXAMPLELOGS

            if [ "$VALGRIND" = "yes" ]
            then
                mkdir $LOGDIR/valgrind/$EXAMPLELOGS
                (
                    cd $CUR_PATH/$PROJECT;
                    for i in `find . -name "vg_*.txt"`; do cp $i $LOGDIR/valgrind/$EXAMPLELOGS; done;
                    chmod -R +r $LOGDIR/valgrind/$EXAMPLELOGS;
                )
            fi

#           cat run.log  >> $RUN_LOG
            sleep 10
        else
            echo "Next Examples"
        fi
    fi
done

cd "$CUR_PATH"

echo "" >> $SUMMARY_LOG
echo "############# Summary of Build ##########"  >> $SUMMARY_LOG
echo "     Examples Run     : $SUM"  >> $SUMMARY_LOG
echo "     Runs passed      : $SUCC"  >> $SUMMARY_LOG
echo "     Runs failed      : $FAIL"  >> $SUMMARY_LOG
echo "#########################################"  >> $SUMMARY_LOG
echo ""  >> $SUMMARY_LOG

echo "RESULTS ......."  >> $SUMMARY_LOG
echo ""  >> $SUMMARY_LOG
cat $RUN_SUMMARY_LOG  >> $SUMMARY_LOG
sleep 10

rm $RUN_SUMMARY_LOG
sleep 5

if [ $FAIL != 0 ]; then

    exit 1
fi


