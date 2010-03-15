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

sleep 7200 &
UNIQID=$!

cd $OSPL_HOME/etc/config

XMLFILE=ospl_no_network.xml
NEWXMLFILE=ospl_no_network_uniq.xml

echo "NEWXMLFILE is $NEWXMLFILE"
echo "UNIQID is $UNIQID"

sed "s@<Name>OpenSpliceV[^<]*</Name>@<Name>oex_$UNIQID</Name>@" < $XMLFILE > $NEWXMLFILE

echo "SET OSPL_URI=file://$OSPL_HOME/etc/config/ospl_no_network_uniq.xml" >> $OSPL_HOME/examples/swap_URI.bat
