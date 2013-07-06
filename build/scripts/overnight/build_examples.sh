. $BASE/build_example_summary_fns 

CUR_PATH=`pwd`

echo " Begin building examples -  `date`"

for PROJECT in $EXAMPLES
do
    if [ -d "$CUR_PATH/$PROJECT" ]
    then
        cd "$CUR_PATH/$PROJECT"

        spec_build="no"

        for test in $SPECIAL_BUILD_FILES
        do
            if [ "$test" = "$PROJECT" ]
            then
                spec_build="yes"
            fi
        done

        if [ "$spec_build" = "no" ] 
        then
            if [ -f "Makefile" ]
            then
                echo " ####  Project: $PROJECT Begin #### " > build.log
                make >> build.log 2>&1
                status=$?
            else
               echo "Not building $PROJECT as make file not found"
            fi
        else
            if [ -f "BUILD" ]
            then
                echo " ####  Project: $PROJECT Begin #### " > build.log
                sh BUILD >> build.log 2>&1
                status=$?
            else
               echo "Not building $PROJECT as BUILD file not found"
            fi
        fi

	create_example_summary $PROJECT
    fi
done

CURRENT_PL_AIX=`uname | grep AIX`
CURRENT_PL_DEBIAN=`hostname | grep debian40`

# Do not build the ospli/testsuite/tests if the platform is AIX or DEBIAN40 as there is a problem with this and we
# don't need to have these tests built as part of the overnights until they are fully integrated/automated
if [ "$CURRENT_PL_AIX" = "" -a "$CURRENT_PL_DEBIAN" = "" ]
then
    spec_build="no"

    echo " ####  Project: Tests Begin #### " > build.log

    cd "$TEST_SRC_DIR" >> build.log 2>&1

    if [ $spec_build = "no" ]
    then
       make >> build.log 2>&1
       status=$?
    else
       sh BUILD >> build.log 2>&1
       status=$?
    fi
    create_example_summary $TEST_SRC_DIR
fi
create_example_final_summary

exit $?