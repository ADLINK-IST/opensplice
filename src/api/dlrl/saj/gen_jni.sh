DDS_PACKAGE=DDS
DLRL_IMPL_PACKAGE=org.opensplice.dds.dlrl
OUT_DIR=$OSPL_OUTER_HOME/src/api/dlrl/saj/c/include

printf "
 " > $OUT_DIR/header.txt

create_impl_header_file()
{
	file=$1
	javafile=$2

	create_header_file $file $DLRL_IMPL_PACKAGE $javafile
}

create_dlrl_header_file()
{
	file=$1
	javafile=$2

	create_header_file $file $DDS_PACKAGE $javafile
}

create_header_file()
{
	file=$1
	package=$2
	javafile=$3

	printf "=> %s..." $file
	javah -classpath $OSPL_HOME/jar/$SPLICE_TARGET/dcpssaj.jar:$OSPL_HOME/jar/$SPLICE_TARGET/dlrlsaj.jar -o $OUT_DIR/$file -jni $package.$javafile
	printf "done\n"
}


cd $OSPL_OUTER_HOME/src/api/dlrl/saj/java/bld/$SPLICE_TARGET
printf "Generating include files, please be patient...\n"
printf "Generating DDS header files...\n"
create_dlrl_header_file DJA_CacheFactory.h CacheFactory
create_dlrl_header_file DJA_IntMap.h IntMap
create_dlrl_header_file DJA_ObjectHome.h ObjectHome
create_dlrl_header_file DJA_ObjectRoot.h ObjectRoot
create_dlrl_header_file DJA_Set.h Set
create_dlrl_header_file DJA_StrMap.h StrMap
printf "DDS header files generation done.\n"

printf "\nGenerating implementation header files...\n"
create_impl_header_file DJA_CacheAccessImpl.h CacheAccessImpl
create_impl_header_file DJA_CacheImpl.h CacheImpl
create_impl_header_file DJA_DCPSUtil.h DCPSUtil
create_impl_header_file DJA_FilterCriterionImpl.h FilterCriterionImpl
create_impl_header_file DJA_QueryCriterionImpl.h QueryCriterionImpl
create_impl_header_file DJA_SelectionImpl.h SelectionImpl
printf "Implementation header files generaration done.\n"

rm -f $OUT_DIR/header.txt
rm -f $OUT_DIR/tmp.txt
printf "\nGeneraration done.\n"
cd -
