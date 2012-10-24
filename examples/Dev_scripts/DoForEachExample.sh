TARGET=""

echo "Do for ContentFilteredTopic:"
echo "Do ${1} ContentFilteredTopic${2} ${3}"
if [ ! "${3}" = "" ] ; then
   TARGET=../dcps/ContentFilteredTopic${3}
fi
$1 ../dcps/ContentFilteredTopic${2}  ${TARGET}

echo "Do for Durability:"
echo "Do ${1} Durability${2} ${3}"
if [ ! "${3}" = "" ] ; then
   TARGET=../dcps/Durability${3}
fi
$1 ../dcps/Durability${2}  ${TARGET}

echo "Do for HelloWorld:"
echo "Do ${1} HelloWorld${2} ${3}"
if [ ! "${3}" = "" ] ; then
   TARGET=../dcps/HelloWorld${3}
fi
$1 ../dcps/HelloWorld${2}  ${TARGET}

echo "Do for Lifecycle:"
echo "Do ${1} Lifecycle${2} ${3}"
if [ ! "${3}" = "" ] ; then
   TARGET=../dcps/Lifecycle${3}
fi
$1 ../dcps/Lifecycle${2}  ${TARGET}

echo "Do for Listener:"
echo "Do ${1} Listener${2} ${3}"
if [ ! "${3}" = "" ] ; then
   TARGET=../dcps/Listener${3}
fi
$1 ../dcps/Listener${2}  ${TARGET}

echo "Do for Ownership:"
echo "Do ${1} Ownership${2} ${3}"
if [ ! "${3}" = "" ] ; then
   TARGET=../dcps/Ownership${3}
fi
$1 ../dcps/Ownership${2}  ${TARGET}

echo "Do for QueryCondition:"
echo "Do ${1} QueryCondition${2} ${3}"
if [ ! "${3}" = "" ] ; then
   TARGET=../dcps/QueryCondition${3}
fi
$1 ../dcps/QueryCondition${2}  ${TARGET}

echo "Do for WaitSet:"
echo "Do ${1} WaitSet${2} ${3}"
if [ ! "${3}" = "" ] ; then
   TARGET=../dcps/WaitSet${3}
fi
$1 ../dcps/WaitSet${2}  ${TARGET}

echo "Do for BuiltInTopics:"
echo "Do ${1} BuiltInTopics${2} ${3}"
if [ ! "${3}" = "" ] ; then
   TARGET=../dcps/BuiltInTopics${3}
fi
$1 ../dcps/BuiltInTopics${2}  ${TARGET}

