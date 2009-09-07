
SUM=0
SUCC=0
FAIL=0

CUR_PATH=`pwd`
echo " Begin"

for PROJECT in $EXAMPLES 
do
    echo " ####  Project: $PROJECT Begin####"
    cd "$CUR_PATH/$PROJECT"
    
    spec_build="no"
    
    for test in $SPECIAL_BUILD_FILES
    do
        if [ "$test" = "$PROJECT" ]; then
            spec_build="yes"
        fi
    done
    
    if [ $spec_build = "no" ]; then
     make
     status=$?
    else
     sh BUILD
     status=$?
    fi
    
    SUM=`expr $SUM + 1`
    if [ $status = 0 ]; then
       SUCC=`expr $SUCC + 1`
       echo "Build successed"
     else
       FAIL=`expr $FAIL + 1`
       echo "Build failed: $status"
     fi  
    
    echo " #### Project: $PROJECT End ####" 
    echo ""
done
cd "$CUR_PATH"

echo "############ Summary of Build:  #############"
echo "    Summary: $SUM"
echo "    Successed: $SUCC"
echo "    Failed: $FAIL"
echo "#########################"
