echo "**************************************************************************************************************************" >> ./MakeAllExamples.log
echo "*** Building all the Examples... *****************************************************************************************" >> ./MakeAllExamples.log
echo "**************************************************************************************************************************" >> ./MakeAllExamples.log

echo "**************************************************************************************************************************"
echo "*** Building all the Examples... *****************************************************************************************"
echo "**************************************************************************************************************************"

rm -f ./MakeAllExamples.log

./MakeOneExample.sh ContentFilteredTopic 
./MakeOneExample.sh QueryCondition 
./MakeOneExample.sh Durability 
./MakeOneExample.sh HelloWorld
./MakeOneExample.sh Lifecycle
./MakeOneExample.sh Listener
./MakeOneExample.sh Ownership
./MakeOneExample.sh WaitSet

echo "**************************************************************************************************************************" >> ./MakeAllExamples.log
echo "**************************************************************************************************************************" >> ./MakeAllExamples.log
echo "Build result is:"														  >> ./MakeAllExamples.log
echo "**************************************************************************************************************************"
echo "**************************************************************************************************************************"
export MAKE_ALL_EXAMPLES_RESULT=""
export MAKE_ALL_EXAMPLES_ERRORS=`grep -i "error" ./MakeAllExamples.log `
if [ "$MAKE_ALL_EXAMPLES_ERRORS" != "" ]; then
  echo "Build failed."                                                                                           		  >> ./MakeAllExamples.log
  echo "Build failed."
  echo "There are some build error(s)!"                                                                                           >> ./MakeAllExamples.log
  echo "Search for \"error\" in this log file..." 		            							  >> ./MakeAllExamples.log
  echo "There are some build error(s)!"
  echo "Search for \"error\" in ./MakeAllExamples.log..."
else
  echo "Build succeeded."                                                                                           		  >> ./MakeAllExamples.log
  echo "Build succeeded."
fi
export MAKE_ALL_EXAMPLES_WARNINGS=`grep -i "warning" ./MakeAllExamples.log `
if [ "$MAKE_ALL_EXAMPLES_WARNINGS" != "" ]; then
  echo "There are some build warning(s)!"                                                                                         >> ./MakeAllExamples.log
  echo "Search for \"warning\" in this log file..."         	           							  >> ./MakeAllExamples.log
  echo "There are some build warning(s)!"
  echo "Search for \"warning\" in ./MakeAllExamples.log..."
else
  echo "No Build warning."
  echo "No Build warning."                                                                                           		  >> ./MakeAllExamples.log
fi

echo "**************************************************************************************************************************" >> ./MakeAllExamples.log
echo "**************************************************************************************************************************"

