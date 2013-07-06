PATH="$JAVA_HOME/bin:$PATH"
PATH="$PATH:$TAO_ROOT/bin:$JACORB_HOME/bin"

CLASSPATH="$CLASSPATH:$JACORB_HOME/lib/endorsed/jacorb.jar:$JACORB_HOME/lib/endorsed/logkit.jar:$JACORB_HOME/lib/idl.jar"

export PATH CLASSPATH

echo "Updating XML:"
echo "OSPL_URI before XMLFILE update is $OSPL_URI"

ospl_unique_domainID > uniqID &
UNIQ_PID=$!

# Wait a little while for the file to be written on slower systems
sleep 5
UNIQID=`cat uniqID`
if [ "$UNIQID" = "" ]
then
    echo "ERROR : unable to get uniqID from ospl_unique_domainID"
    exit 1
fi
echo "UNIQID is $UNIQID"

if [ "$EXRUNTYPE" = "shm" ]
then
    XMLFILE=`echo $OSPL_URI | sed 's@file://@@' | sed 's/ospl.xml$/ospl_shmem_no_network.xml/'`
    NEWXMLFILE=`echo $XMLFILE | sed 's/_no_network.xml$/_no_network_uniq.xml/'`
    sed -e "s@<Name>ospl_[^<]*</Name>@<Name>oex_$UNIQID</Name>@" \
        -e "s@<Id>0</Id>@<Id>$UNIQID</Id>@" \
        -e "s@<PortNr>50000</PortNr>@<PortNr>Auto</PortNr>@" < $XMLFILE > $NEWXMLFILE
    OSPL_URI=`echo $OSPL_URI | sed 's/ospl.xml/ospl_shmem_no_network_uniq.xml/'`
else
    XMLFILE=`echo $OSPL_URI | sed 's@file://@@' | sed 's/ospl.xml$/ospl_sp_ddsi.xml/'`
    NEWXMLFILE=`echo $XMLFILE | sed 's/ospl_sp_ddsi.xml$/ospl_sp_ddsi_uniq.xml/'`

    # check that MulticastRecvNetworkInterfaceAddresses is not already present in the xml file:
    grep MulticastRecvNetworkInterfaceAddresses $XMLFILE
    if [ $? = 0 ]
    then
        echo "ERROR : MulticastRecvNetworkInterfaceAddresses already exists"

        exit 1;
    fi

    sed -e "s@<Name>ospl_[^<]*</Name>@<Name>oex_$UNIQID</Name>@" \
        -e "s@<Id>0</Id>@<Id>$UNIQID</Id>@" \
        -e 's@<NetworkInterfaceAddress>AUTO</NetworkInterfaceAddress>@<NetworkInterfaceAddress>127.0.0.1</NetworkInterfaceAddress>\
           <MulticastRecvNetworkInterfaceAddresses>127.0.0.1</MulticastRecvNetworkInterfaceAddresses>@'  < $XMLFILE > $NEWXMLFILE
    OSPL_URI=`echo $OSPL_URI | sed 's/ospl.xml/ospl_sp_ddsi_uniq.xml/'`

    # sanity check that the sed'ing worked (i.e. that the strings existed in the first place)
    grep MulticastRecvNetworkInterfaceAddresses $NEWXMLFILE
    if [ $? = 1 ]
    then
        echo "ERROR : MulticastRecvNetworkInterfaceAddresses does not exist"
        exit 1;
    fi
fi

echo "XMLFILE is $XMLFILE"
echo "OSPL_URI is $OSPL_URI"
echo "NEWXMLFILE is $NEWXMLFILE"

ODBCINI="$ODBCHOME/etc/odbc.ini"
ODBCINST="$ODBCHOME/etc/odbcinst.ini"
ODBC_MSSQL_SERVER="10.1.0.189"
ODBC_MYSQL_SERVER="10.1.0.191"
ODBCSYSINI="$ODBCHOME/etc"
LD_LIBRARY_PATH="$ODBCHOME/lib:.:$LD_LIBRARY_PATH"

# Need to do this for solaris for running the dbmsconnect example
if [ "$IS_STUDIO12" != 0 ]
then
   LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/gcc-3.4.6/lib"
fi

export  ODBCINI ODBCINST ODBC_MSSQL_SERVER ODBC_MYSQL_SERVER ODBCSYSINI LD_LIBRARY_PATH

SUM=0
SUCC=0
FAIL=0
RUN_SUMMARY_LOG=$LOGDIR/examples/run_$EXRUNTYPE/run_results_summary.txt
RUN_LOG=$LOGDIR/examples/run_$EXRUNTYPE/run_results.txt
SUMMARY_LOG=$LOGDIR/examples/run_$EXRUNTYPE/examples.log
TOTALS_LOG=$LOGDIR/examples/run_$EXRUNTYPE/totals.log

CUR_PATH=`pwd`
echo " Begin running examples - `date`"

for PROJECT in $EXAMPLES
do
    if [ -f "$CUR_PATH/$PROJECT/RUN" ]
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
            #Create a directory using the project directory location, ommitting the slashes
            EXAMPLEDIR=`echo $PROJECT | sed -e 's/standalone/SA/' -e 's/CORBA/C/' -e 's/Java/J/' -e 's/C++/CPP/' -e 's/JacORB//' -e 's/OpenFusion//' -e 's/\///g'`


            sh RUN $EXRUNTYPE >> run.log 2>&1
            status=$?

            SUM=`expr $SUM + 1`
            if [ $status = 0 ]
            then
               # ignore the print out from ospl describing the location of the error log
               # also ignore the "Killed" message from when spliced is terminated by the scripts (single process mode only)
               if [ -n "`egrep -i '(segmentation|glibc detected|killed|timeout|not found|No such file or directory|NoClassDefFoundError|Assertion failed|Creation of kernel failed|error|Failed)' $CUR_PATH/$PROJECT/run.log | grep -v '^Error log :' | grep -v 'Printer Error' | grep -v 'singleProcessResult.txt' `" ]
               then               
                   FAIL=`expr $FAIL + 1`

                   ERROR=`grep -ci "error" $CUR_PATH/$PROJECT/run.log`

                   SEGMENTATIONFAULTS=`grep -ci "segmentation" $CUR_PATH/$PROJECT/run.log`
                   NOTFOUND=`grep -ci "not found" $CUR_PATH/$PROJECT/run.log`
                   ASSERTIONFAILED=`grep -ci "assertion failed" $CUR_PATH/$PROJECT/run.log`
                   NOCLASSDEFFOUND=`grep -ci "NoClassDefFoundError" $CUR_PATH/$PROJECT/run.log`
                   CREATIONOFKERNEL=`grep -ci "creation of kernel failed" $CUR_PATH/$PROJECT/run.log`
                   TIMEOUTS=`grep -ci "timeout" $CUR_PATH/$PROJECT/run.log`
                   KILLED=`grep -ci "killed" $CUR_PATH/$PROJECT/run.log`
                   GLIBC=`grep -ci "glibc" $CUR_PATH/$PROJECT/run.log`
                   FAILED=`grep -ci "Failed" $CUR_PATH/$PROJECT/run.log`
                   MESSAGE=""

                   if [ $SEGMENTATIONFAULTS != 0 ]
                   then
                      MESSAGE="Segmentation Faults occurred"
                   fi

                   if [ $NOTFOUND != 0 ]
                   then
                      MESSAGE="$MESSAGE $NOTFOUND not found occurrences"
                   fi

                   if [ $NOCLASSDEFFOUND != 0 ]
                   then
                      MESSAGE="$MESSAGE NoClassDefFound errors"
                   fi

                   if [ $NOCLASSDEFFOUND != 0 ]
                   then
                      MESSAGE="$MESSAGE $ERROR errors occurred"
                   fi

                   if [ $CREATIONOFKERNEL != 0 ]
                   then
                      MESSAGE="$MESSAGE Creation of kernel failures"
                   fi

                   if [ $TIMEOUTS != 0 ]
                   then
                      MESSAGE="$MESSAGE $TIMEOUTS timeout(s) occurred"
                   fi

                   if [ $KILLED != 0 ]
                   then
                      MESSAGE="$MESSAGE Example killed"
                   fi

                   if [ $ASSERTIONFAILED != 0 ]
                   then
                      MESSAGE="$MESSAGE $ASSERTIONFAILED Assertion Failed occurrences"
                   fi

                   if [ $GLIBC != 0 ]
                   then
                      MESSAGE="$MESSAGE $GLIBC glibc found"
                   fi

                   if [ $FAILED != 0 ]
                   then
                      MESSAGE="$MESSAGE Example failed"
                   fi

                   echo "$PROJECT FAILED $MESSAGE" >> run.log
                   echo "$EXAMPLEDIR FAILED $MESSAGE" >> $RUN_SUMMARY_LOG
               elif [ -f $CUR_PATH/$PROJECT/ospl-error.log ]
               then
                    FAIL=`expr $FAIL + 1`
                    echo "$PROJECT FAILED ospl-error.log found " >> run.log
                    echo "$EXAMPLEDIR FAILED ospl-error.log found " >> $RUN_SUMMARY_LOG
                elif [ -n "`egrep 'WARNING' $CUR_PATH/$PROJECT/ospl-info.log`" ]
                then
                    # Add extra output to indicate how many of the warnings are "Missed heartbeats"
                    # so we can quickly identify new warnings appearing
                    WARNINGS=`grep -ci "WARNING" $CUR_PATH/$PROJECT/ospl-info.log`
                    MISSEDHEARTBEAT=`grep -ci "Missed heartbeat" $CUR_PATH/$PROJECT/ospl-info.log`
                    FAIL=`expr $FAIL + 1`
                    echo "$PROJECT FAILED - $WARNINGS WARNINGs found in ospl-info.log" >> run.log
                    echo "$EXAMPLEDIR FAILED - $WARNINGS WARNINGs found in ospl-info.log ($MISSEDHEARTBEAT Missed heartbeat warnings)" >> $RUN_SUMMARY_LOG
                else
                    SUCC=`expr $SUCC + 1`
                    echo " ### Run $PROJECT PASSED ### " >> run.log
                    echo " $EXAMPLEDIR PASSED" >> $RUN_SUMMARY_LOG
                fi
            else
                FAIL=`expr $FAIL + 1`
            echo " ### Run $PROJECT FAILED with status: $status ### " >> run.log
            echo "$EXAMPLEDIR FAILED with status: $status" >> $RUN_SUMMARY_LOG
            fi

            echo " ### Project: $PROJECT End ### " >> run.log
            echo "" >> run.log

            mkdir $LOGDIR/examples/run_$EXRUNTYPE/$EXAMPLEDIR
            cp $CUR_PATH/$PROJECT/*.log $LOGDIR/examples/run_$EXRUNTYPE/$EXAMPLEDIR
            cp $CUR_PATH/$PROJECT/*Result*.txt $LOGDIR/examples/run_$EXRUNTYPE/$EXAMPLEDIR
            if [ "$VALGRIND" = "yes" ]
            then
                (
                    VGLOGPATH=$LOGDIR/valgrind_${EXRUNTYPE}/$EXAMPLEDIR;
                    mkdir $VGLOGPATH;
                    find $CUR_PATH/$PROJECT -name "vg_*.txt" -type f -print0 \
                        | xargs -0 -I @ cp @ $VGLOGPATH;
                    chmod -R +r $VGLOGPATH
                )
            fi

            cat run.log  >> $RUN_LOG
            sleep 10
        else
            echo "Next Examples"
        fi
    fi
done

echo "Killing unique domainID generator"
kill $UNIQ_PID

echo "Examples Run = $SUM" > $TOTALS_LOG
echo "Examples Passed = $SUCC" >> $TOTALS_LOG
echo "Examples Failed = $FAIL" >> $TOTALS_LOG

if [ $FAIL != 0 ]; then

    exit 1
fi

