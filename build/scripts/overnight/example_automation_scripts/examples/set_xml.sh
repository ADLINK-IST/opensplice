# This script is like this because I could not find a simple way to do this
# for windows.  Basically we cannot set the DDS environment until we are in
# a cmd.exe window.  Once there the swapping of the files and the chaning 
# the contents cannot be done easily in a batch file.
# So we need to use a shell script.  But then
# the locations are pointing to C:\ locations which don't work with the shell
# script.  The objective seems to be to point the OSPL_URI to a unique file
# that has the OSPL daemon name changed to something unique.  As the examples
# run at the end of the overnights and this swap is only taking place for the
# examples then I think we are safe to do it this way for now.  We may need to
# find a better way if/when we start testing multi-node.  Or if some clever 
# person can find a better way to do this.  BTW this file is for internal use
# only as part of the examples test run

echo "Updating XML:"
echo "OSPL_URI before update is $OSPL_URI"

ospl_unique_domainID > uniqID &
UNIQ_PID=$!
echo $UNIQ_PID > uniqIDPID
# sleep here because ospl_unique_domainID may not immediately output to uniqID
sleep 5
UNIQID=`cat uniqID`
if [ "$UNIQID" = "" ]
then
    echo "ERROR : unable to get uniqID from ospl_unique_domainID"
    exit 1
fi
echo "UNIQID is $UNIQID"

cd "$OSPL_HOME/etc/config"

# Generate the XML configuration for running the examples : either shared memory or single process
if [ "$EXRUNTYPE" = "shm" ]
then
    XMLFILE=ospl_shmem_no_network.xml
    NEWXMLFILE=ospl_shmem_no_network_uniq.xml

    sed -e "s@<Name>ospl_[^<]*</Name>@<Name>oex_$UNIQID</Name>@" \
        -e "s@<Id>0</Id>@<Id>$UNIQID</Id>@" < $XMLFILE > $NEWXMLFILE

    echo "SET OSPL_URI=file://$OSPL_HOME/etc/config/ospl_shmem_no_network_uniq.xml" >> $OSPL_HOME/examples/swap_URI.bat
else
    XMLFILE=ospl_sp_ddsi.xml
    NEWXMLFILE=ospl_sp_ddsi_uniq.xml

    # check that MulticastRecvNetworkInterfaceAddresses is not already present in the xml file:
    grep MulticastRecvNetworkInterfaceAddresses $XMLFILE
    if [ $? = 0 ]
    then
        echo "ERROR : MulticastRecvNetworkInterfaceAddresses already exists"
        exit 1;
    fi

    sed -e "s@<Name>ospl_[^<]*</Name>@<Name>oex_$UNIQID</Name>@" \
        -e "s@<Id>0</Id>@<Id>$UNIQID</Id>@" \
        -e 's@<NetworkInterfaceAddress>AUTO</NetworkInterfaceAddress>@<NetworkInterfaceAddress>127.0.0.1</NetworkInterfaceAddress>\n       <MulticastRecvNetworkInterfaceAddresses>127.0.0.1</MulticastRecvNetworkInterfaceAddresses>@'  < $XMLFILE > $NEWXMLFILE

    # sanity check that the sed'ing worked (i.e. that the strings existed in the first place)
    grep MulticastRecvNetworkInterfaceAddresses $NEWXMLFILE
    if [ $? = 1 ]
    then
        echo "ERROR : MulticastRecvNetworkInterfaceAddresses does not exist"
        exit 1;
    fi

    echo "SET OSPL_URI=file://$OSPL_HOME/etc/config/ospl_sp_ddsi_uniq.xml" >> $OSPL_HOME/examples/swap_URI.bat
fi

echo "XMLFILE is $XMLFILE"
echo "NEWXMLFILE is $NEWXMLFILE"
echo "OSPL_URI is $OSPL_URI"
