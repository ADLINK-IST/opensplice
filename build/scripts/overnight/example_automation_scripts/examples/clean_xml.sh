echo "Killing unique domainID generator"

UNIQ_PID=`cat uniqIDPID`
kill $UNIQ_PID
