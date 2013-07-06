echo "**************************************************************************************************************************" > ./MakeAllExamples.log
echo "*** Building all the Examples... *****************************************************************************************" >> ./MakeAllExamples.log
echo "**************************************************************************************************************************" >> ./MakeAllExamples.log
echo "**************************************************************************************************************************"

echo "**************************************************************************************************************************"
echo "*** Building in C... *****************************************************************************************************"
echo "**************************************************************************************************************************"
echo "**************************************************************************************************************************"
echo "**************************************************************************************************************************"

rm -f ./MakeAllExamples.log

./MakeOneExample.sh ../dcps/ContentFilteredTopic/C/Standalone/Posix "../../../../../Dev_scripts" 
./MakeOneExample.sh ../dcps/QueryCondition/C/Standalone/Posix "../../../../../Dev_scripts"
./MakeOneExample.sh ../dcps/Durability/C/Standalone/Posix "../../../../../Dev_scripts"
./MakeOneExample.sh ../dcps/HelloWorld/C/Standalone/Posix "../../../../../Dev_scripts"
./MakeOneExample.sh ../dcps/Lifecycle/C/Standalone/Posix "../../../../../Dev_scripts"
./MakeOneExample.sh ../dcps/Listener/C/Standalone/Posix "../../../../../Dev_scripts"
./MakeOneExample.sh ../dcps/Ownership/C/Standalone/Posix "../../../../../Dev_scripts"
./MakeOneExample.sh ../dcps/WaitSet/C/Standalone/Posix "../../../../../Dev_scripts"

echo "**************************************************************************************************************************"
echo "*** Building in C++... ***************************************************************************************************"
echo "**************************************************************************************************************************"
echo "**************************************************************************************************************************"
echo "**************************************************************************************************************************"

./MakeOneExample.sh ../dcps/ContentFilteredTopic/C++/Standalone/Posix "../../../../../Dev_scripts" 
./MakeOneExample.sh ../dcps/QueryCondition/C++/Standalone/Posix "../../../../../Dev_scripts"
./MakeOneExample.sh ../dcps/Durability/C++/Standalone/Posix "../../../../../Dev_scripts"
./MakeOneExample.sh ../dcps/HelloWorld/C++/Standalone/Posix "../../../../../Dev_scripts"
./MakeOneExample.sh ../dcps/Lifecycle/C++/Standalone/Posix "../../../../../Dev_scripts"
./MakeOneExample.sh ../dcps/Listener/C++/Standalone/Posix "../../../../../Dev_scripts"
./MakeOneExample.sh ../dcps/Ownership/C++/Standalone/Posix "../../../../../Dev_scripts"
./MakeOneExample.sh ../dcps/WaitSet/C++/Standalone/Posix "../../../../../Dev_scripts"

echo "**************************************************************************************************************************"
echo "*** Building in C++ Corba/OpenFusion... **********************************************************************************"
echo "**************************************************************************************************************************"

./MakeOneExample.sh ../dcps/HelloWorld/C++/Corba/OpenFusion/Posix "../../../../../../Dev_scripts"

echo "**************************************************************************************************************************"
echo "*** Building in Java... **************************************************************************************************"
echo "**************************************************************************************************************************"
echo "**************************************************************************************************************************"
echo "**************************************************************************************************************************"

./MakeOneExample_Java.sh ../dcps/ContentFilteredTopic/Java/Standalone/Posix "../../../../../Dev_scripts" 
./MakeOneExample_Java.sh ../dcps/QueryCondition/Java/Standalone/Posix "../../../../../Dev_scripts"
./MakeOneExample_Java.sh ../dcps/Durability/Java/Standalone/Posix "../../../../../Dev_scripts"
./MakeOneExample_Java.sh ../dcps/HelloWorld/Java/Standalone/Posix "../../../../../Dev_scripts"
./MakeOneExample_Java.sh ../dcps/Lifecycle/Java/Standalone/Posix "../../../../../Dev_scripts"
./MakeOneExample_Java.sh ../dcps/Listener/Java/Standalone/Posix "../../../../../Dev_scripts"
./MakeOneExample_Java.sh ../dcps/Ownership/Java/Standalone/Posix "../../../../../Dev_scripts"
./MakeOneExample_Java.sh ../dcps/WaitSet/Java/Standalone/Posix "../../../../../Dev_scripts"

echo "**************************************************************************************************************************"
echo "*** Building in Java Corba/JacORB... *************************************************************************************"
echo "**************************************************************************************************************************"

./MakeOneExample_Java.sh ../dcps/HelloWorld/Java/Corba/JacORB/Posix "../../../../../../Dev_scripts"

echo "**************************************************************************************************************************" >> ./MakeAllExamples.log
echo "**************************************************************************************************************************" >> ./MakeAllExamples.log
echo "Build result is:"                                                                                                           >> ./MakeAllExamples.log
echo "**************************************************************************************************************************"
echo "**************************************************************************************************************************"
export MAKE_ALL_EXAMPLES_ERRORS=`grep -i "error" ./MakeAllExamples.log `
if [ "$MAKE_ALL_EXAMPLES_ERRORS" != "" ]; then
  echo "Build failed."                                                                                                            >> ./MakeAllExamples.log
  echo "Build failed."
  echo "There are some build error(s)!"                                                                                           >> ./MakeAllExamples.log
  echo "Search for \"error\" in this log file..."                                                                                 >> ./MakeAllExamples.log
  echo "There are some build error(s)!"
  echo "Search for \"error\" in ./MakeAllExamples.log..."
else
  echo "Build succeeded."                                                                                                         >> ./MakeAllExamples.log
  echo "Build succeeded."
fi
export MAKE_ALL_EXAMPLES_WARNINGS=`grep -i "warning" ./MakeAllExamples.log `
if [ "$MAKE_ALL_EXAMPLES_WARNINGS" != "" ]; then
  echo "There are some build warning(s)!"                                                                                         >> ./MakeAllExamples.log
  echo "Search for \"warning\" in this log file..."                                                                               >> ./MakeAllExamples.log
  echo "There are some build warning(s)!"
  echo "Search for \"warning\" in ./MakeAllExamples.log..."
else
  echo "No Build warning."
  echo "No Build warning."                                                                                                        >> ./MakeAllExamples.log
fi

echo "**************************************************************************************************************************" >> ./MakeAllExamples.log
echo "**************************************************************************************************************************"

