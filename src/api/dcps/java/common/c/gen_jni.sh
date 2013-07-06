DCPS_IMPL_PACKAGE=org.opensplice.dds.dcps
DCPS_PACKAGE=DDS
OUT_DIR=$OSPL_HOME/src/api/dcps/saj/c/include

printf "
 " > $OUT_DIR/header.txt

create_impl_header_file()
{
	file=$1
	javafile=$2
	
	create_header_file $file $DCPS_IMPL_PACKAGE $javafile
}

create_dcps_header_file()
{
	file=$1
	javafile=$2
	
	create_header_file $file $DCPS_PACKAGE $javafile
}

create_header_file()
{
	file=$1
	package=$2
	javafile=$3
	
	printf "=> %s..." $file
	javah -o $OUT_DIR/$file -jni $package.$javafile
	insert_pvcs_header $file
	printf "done\n"
}



insert_pvcs_header()
{
	file=$1
	
	cat $OUT_DIR/header.txt > $OUT_DIR/tmp.txt
	cat $OUT_DIR/$file >> $OUT_DIR/tmp.txt
	mv  $OUT_DIR/tmp.txt $OUT_DIR/$file
}

cd $OSPL_HOME/src/api/dcps/saj/java/bld/$SPLICE_TARGET
printf "Generating include files, please be patient...\n"
printf "Generating DDS header files...\n"
create_dcps_header_file saj_domainParticipantFactory.h DomainParticipantFactory
create_dcps_header_file saj_guardCondition.h GuardCondition
create_dcps_header_file saj_waitSet.h WaitSet
create_dcps_header_file saj_errorInfo.h ErrorInfo
printf "DDS header files generation done.\n"

printf "\nGenerating implementation header files...\n"
create_impl_header_file saj_condition.h ConditionImpl
create_impl_header_file saj_contentFilteredTopic.h ContentFilteredTopicImpl
create_impl_header_file saj_dataReader.h DataReaderImpl
create_impl_header_file saj_fooDataReader.h FooDataReaderImpl
create_impl_header_file saj_dataReaderView.h DataReaderViewImpl
create_impl_header_file saj_fooDataReaderView.h FooDataReaderViewImpl
create_impl_header_file saj_dataWriter.h DataWriterImpl
create_impl_header_file saj_fooDataWriter.h FooDataWriterImpl
create_impl_header_file saj_domainParticipant.h DomainParticipantImpl
create_impl_header_file saj_entity.h EntityImpl
create_impl_header_file saj_multiTopic.h MultiTopicImpl
create_impl_header_file saj_publisher.h PublisherImpl
create_impl_header_file saj_queryCondition.h QueryConditionImpl
create_impl_header_file saj_readCondition.h ReadConditionImpl
create_impl_header_file saj_statusCondition.h StatusConditionImpl
create_impl_header_file saj_subscriber.h SubscriberImpl
create_impl_header_file saj_topicDescription.h TopicDescriptionImpl
create_impl_header_file saj_topic.h TopicImpl
create_impl_header_file saj_domain.h DomainImpl
create_impl_header_file saj_typeSupport.h TypeSupportImpl
create_impl_header_file saj_fooTypeSupport.h FooTypeSupportImpl
printf "Implementation header files generaration done.\n"

rm -f $OUT_DIR/header.txt
rm -f $OUT_DIR/tmp.txt
printf "\nGeneraration done.\n"
cd -
