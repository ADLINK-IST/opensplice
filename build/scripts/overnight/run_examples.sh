#set -x

. $BASE/example_results_fns 

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


    if [ -n "$UNIQE_MC_ADDRESS" ]
    then 
        grep SPDPMulticastAddress $XMLFILE
        if [ $? = 0 ]
        then
            echo "ERROR : SPDPMulticastAddress already exists"  
            exit 1;
        fi
        sed -e "s@<Name>ospl_[^<]*</Name>@<Name>oex_$UNIQID</Name>@" \
            -e "s@<Id>0</Id>@<Id>$UNIQID</Id>@" \
            -e "s@</DDSI2Service>@<Discovery><SPDPMulticastAddress>$UNIQE_MC_ADDRESS</SPDPMulticastAddress></Discovery></DDSI2Service>@" < $XMLFILE > $NEWXMLFILE
        grep SPDPMulticastAddress $NEWXMLFILE
        if [ $? = 1 ]
        then
            echo "ERROR : SPDPMulticastAddress does not exist"
            exit 1;
        fi
    else
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
        # sanity check that the sed'ing worked (i.e. that the strings existed in the first place)
        grep MulticastRecvNetworkInterfaceAddresses $NEWXMLFILE
        if [ $? = 1 ]
        then
            echo "ERROR : MulticastRecvNetworkInterfaceAddresses does not exist"
            exit 1;
        fi
    fi
    OSPL_URI=`echo $OSPL_URI | sed 's/ospl.xml/ospl_sp_ddsi_uniq.xml/'`
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

export  ODBCINI ODBCINST ODBC_MSSQL_SERVER ODBC_MYSQL_SERVER ODBCSYSINI LD_LIBRARY_PATH OSPL_URI

SUM=0
SUCC=0
FAIL=0
RUN_SUMMARY_LOG=$LOGDIR/examples/run_$EXRUNTYPE/run_results_summary.txt
RUN_LOG=$LOGDIR/examples/run_$EXRUNTYPE/run_results.txt
SUMMARY_LOG=$LOGDIR/examples/run_$EXRUNTYPE/examples.log
TOTALS_LOG=$LOGDIR/examples/run_$EXRUNTYPE/totals.log

export RUN_SUMMARY_LOG RUN_LOG SUMMARY_LOG TOTALS_LOG

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

        #Clean up any previous run logs
        #single process and shared memory run after one another
        if [ -f "ospl-error.log" ]
        then
            rm ospl-error.log
        fi
        if [ -f "ospl-info.log" ]
        then
            rm ospl-info.log
        fi
        if [ -f "run.log" ]
        then
            rm run.log
        fi

        #Create a run.log so all output is captured, for some reason the durability
        #examples do not get a run.log until the result is analysed.  The output from
        #the durability examples actually goes to .txt files
        touch run.log
        echo " ### Project: $PROJECT Begin ### " >> run.log

        if [ $run = "yes" ];
        then
            #Create a directory using the project directory location, ommitting the slashes
            EXAMPLEDIR=`echo $PROJECT | sed -e 's/standalone/SA/' -e 's/CORBA/C/' -e 's/Java/J/' -e 's/C++/CPP/' -e 's/JacORB//' -e 's/OpenFusion//' -e 's/\///g'`

            export EXAMPLEDIR

            sh RUN $EXRUNTYPE >> run.log 2>&1
            status=$?

            check_example_result $PROJECT 

            mkdir $LOGDIR/examples/run_$EXRUNTYPE/$EXAMPLEDIR
            cp $CUR_PATH/$PROJECT/*.log $LOGDIR/examples/run_$EXRUNTYPE/$EXAMPLEDIR

            if ls $CUR_PATH/$PROJECT/*Result*.txt &> /dev/null; then
               cp $CUR_PATH/$PROJECT/*Result*.txt $LOGDIR/examples/run_$EXRUNTYPE/$EXAMPLEDIR
            fi

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

create_example_results_summary

exit $?
