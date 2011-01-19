TMP_FILE=$OSPL_HOME/src/api/cm/jni/tmp.txt
OUT_FILE=$OSPL_HOME/src/api/cm/jni/include/cmj_factory.h

printf "
 " > $TMP_FILE

cd $OSPL_HOME/src/api/cm/java/bld/$SPLICE_TARGET
javah -o $OUT_FILE -jni org.opensplice.cm.com.JniCommunicator
cat $OUT_FILE >> $TMP_FILE
mv $TMP_FILE $OUT_FILE
cd -
