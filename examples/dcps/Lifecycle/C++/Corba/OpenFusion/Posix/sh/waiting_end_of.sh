SUB_PID=$1
APPLI=$2
MAXITER=150
NBMN=$(($MAXITER/30))
echo === Waiting end of "$APPLI" - pid : $SUB_PID...
if [ "$SUB_PID" != "" ]; then
   export NAME_GREP=""
   NAME_GREP=`ps faux | grep $SUB_PID  | grep -v grep | grep -v waiting_end_of | grep "$APPLI" `;
   echo "First NAME_GREP ===> $NAME_GREP"

   ITER=0
   echo === Max iteration = $MAXITER
   while [ "$NAME_GREP" != "" -a $ITER -lt $MAXITER ] ;
   do
      NAME_GREP=`ps faux | grep $SUB_PID | grep -v grep | grep -v waiting_end_of | grep "$APPLI" ` 
      echo "=== Waiting for $APPLI Subscriber results..."
      sleep 2
      ITER=$(($ITER+1))
   done
   if [ $ITER -ge $MAXITER ]; then
      echo "*** Error : $APPLI Timed out (not stopping after $NBMN mn)"
      echo "*** Killing $APPLI  ..."
      kill -9 $SUB_PID
      exit
   fi
fi
