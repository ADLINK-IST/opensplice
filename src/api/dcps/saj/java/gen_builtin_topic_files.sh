# This hard code won't necessarily do what it
# says on the tin if you change IDL contents. Beware ! @todo

# Added $OSPL_HOME/src/api/dcps/saj/java/header.txt - previously ""

cygwin=false;
case "`uname`" in
  CYGWIN*) cygwin=true;
esac
 
OUT_DIR=$OSPL_HOME/src/api/dcps/saj/java

insert_header_and_move()
{
    file=$1

    printf "Generating %s..." $file
    cat $OUT_DIR/header.txt >$OUT_DIR/tmp.txt
    cat $OUT_DIR/$file >> $OUT_DIR/tmp.txt
    if $cygwin; then
        dos2unix $OUT_DIR/tmp.txt # remove spurious ^M's
    fi
    rm -f $OUT_DIR/$file
    mv $OUT_DIR/tmp.txt $OUT_DIR/code/$file
    printf "done\n"
}

cd $OUT_DIR
idlpp -l java -S code/dds_builtinTopics.idl

FILES="\
    DDS/ParticipantBuiltinTopicData.java \
    DDS/ParticipantBuiltinTopicDataHolder.java \
    DDS/ParticipantBuiltinTopicDataMetaHolder.java \
    DDS/ParticipantBuiltinTopicDataTypeSupport.java \
    DDS/ParticipantBuiltinTopicDataTypeSupportHolder.java \
    DDS/ParticipantBuiltinTopicDataTypeSupportHelper.java \
    DDS/ParticipantBuiltinTopicDataTypeSupportOperations.java \
    DDS/ParticipantBuiltinTopicDataDataReader.java \
    DDS/ParticipantBuiltinTopicDataDataReaderHolder.java \
    DDS/ParticipantBuiltinTopicDataDataReaderHelper.java \
    DDS/ParticipantBuiltinTopicDataDataReaderOperations.java \
    DDS/ParticipantBuiltinTopicDataDataReaderImpl.java \
    DDS/ParticipantBuiltinTopicDataSeqHolder.java \
    DDS/TopicBuiltinTopicData.java \
    DDS/TopicBuiltinTopicDataHolder.java \
    DDS/TopicBuiltinTopicDataMetaHolder.java \
    DDS/TopicBuiltinTopicDataTypeSupport.java \
    DDS/TopicBuiltinTopicDataTypeSupportHolder.java \
    DDS/TopicBuiltinTopicDataTypeSupportHelper.java \
    DDS/TopicBuiltinTopicDataTypeSupportOperations.java \
    DDS/TopicBuiltinTopicDataDataReader.java \
    DDS/TopicBuiltinTopicDataDataReaderHolder.java \
    DDS/TopicBuiltinTopicDataDataReaderHelper.java \
    DDS/TopicBuiltinTopicDataDataReaderOperations.java \
    DDS/TopicBuiltinTopicDataDataReaderImpl.java \
    DDS/TopicBuiltinTopicDataSeqHolder.java \
    DDS/PublicationBuiltinTopicData.java \
    DDS/PublicationBuiltinTopicDataHolder.java \
    DDS/PublicationBuiltinTopicDataMetaHolder.java \
    DDS/PublicationBuiltinTopicDataTypeSupport.java \
    DDS/PublicationBuiltinTopicDataTypeSupportHolder.java \
    DDS/PublicationBuiltinTopicDataTypeSupportHelper.java \
    DDS/PublicationBuiltinTopicDataTypeSupportOperations.java \
    DDS/PublicationBuiltinTopicDataDataReader.java \
    DDS/PublicationBuiltinTopicDataDataReaderHolder.java \
    DDS/PublicationBuiltinTopicDataDataReaderHelper.java \
    DDS/PublicationBuiltinTopicDataDataReaderOperations.java \
    DDS/PublicationBuiltinTopicDataDataReaderImpl.java \
    DDS/PublicationBuiltinTopicDataSeqHolder.java \
    DDS/SubscriptionBuiltinTopicData.java \
    DDS/SubscriptionBuiltinTopicDataHolder.java \
    DDS/SubscriptionBuiltinTopicDataMetaHolder.java \
    DDS/SubscriptionBuiltinTopicDataTypeSupport.java \
    DDS/SubscriptionBuiltinTopicDataTypeSupportHolder.java \
    DDS/SubscriptionBuiltinTopicDataTypeSupportHelper.java \
    DDS/SubscriptionBuiltinTopicDataTypeSupportOperations.java \
    DDS/SubscriptionBuiltinTopicDataDataReader.java \
    DDS/SubscriptionBuiltinTopicDataDataReaderHolder.java \
    DDS/SubscriptionBuiltinTopicDataDataReaderHelper.java \
    DDS/SubscriptionBuiltinTopicDataDataReaderOperations.java \
    DDS/SubscriptionBuiltinTopicDataDataReaderImpl.java \
    DDS/SubscriptionBuiltinTopicDataSeqHolder.java"

for i in $FILES; do
    insert_header_and_move $i
done

# LOOK !!!!!!!!   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
printf "If you're seeing this message THIS HASN'T BEEN FIXED.\n"

rm -rf $OUT_DIR/DDS/*.java
rmdir $OUT_DIR/DDS
