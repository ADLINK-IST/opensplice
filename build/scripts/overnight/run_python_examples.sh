#set -x

CURRENT_PL_DARWIN=`uname | grep Darwin`

if [ "$JAVA_HOME" != "" ]
then
    PATH="$JAVA_HOME/bin:$PATH"
fi

if [ -n "$PERC_HOME" ]
then
   PATH="$PERC_HOME/bin:$PATH"
fi

export PATH

#PATH="$PATH:$TAO_ROOT/bin:$JACORB_HOME/bin"
#CLASSPATH="$CLASSPATH:$JACORB_HOME/lib/endorsed/jacorb.jar:$JACORB_HOME/lib/endorsed/logkit.jar:$JACORB_HOME/lib/idl.jar"
#export PATH CLASSPATH

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

DBSIZE="10485760"

if [ "$CURRENT_PL_DARWIN" != "" ]
then
    DBSIZE="0x500000"
fi

if [ "$EXRUNTYPE" = "shm" ]
then
    XMLFILE=`echo $OSPL_URI | sed 's@file://@@' | sed 's/ospl.xml$/ospl_shmem_no_network.xml/'`
    NEWXMLFILE=`echo $XMLFILE | sed 's/_no_network.xml$/_no_network_uniq.xml/'`
    
    # Mac OS builds seem to have a problem with the \ in the sed command 
    if [ "$CURRENT_PL_DARWIN" != "" ]
    then
        sed -e "s@<Name>ospl_[^<]*</Name>@<Name>oex_$UNIQID</Name>@" -e "s@<Id>0</Id>@<Id>$UNIQID</Id>@" -e "s@<PortNr>50000</PortNr>@<PortNr>Auto</PortNr>@" -e "s@<Size>10485760</Size>@<Size>$DBSIZE</Size>@" < $XMLFILE > $NEWXMLFILE
    else
        sed -e "s@<Name>ospl_[^<]*</Name>@<Name>oex_$UNIQID</Name>@" \
            -e "s@<Id>0</Id>@<Id>$UNIQID</Id>@" \
            -e "s@<PortNr>50000</PortNr>@<PortNr>Auto</PortNr>@" \
            -e "s@<Size>10485760</Size>@<Size>$DBSIZE</Size>@" < $XMLFILE > $NEWXMLFILE
    fi

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

CUR_PATH=`pwd`
echo " Begin running examples - `date`"

which python

env | sort

cd $BASE/python

python runExample.py -a

echo "Killing unique domainID generator"
kill $UNIQ_PID

exit $?
