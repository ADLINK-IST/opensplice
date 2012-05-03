echo "=== Removing all subResult.txt"
rm -f subResult_*.txt

echo "===================="
echo "=== Scenario 3.1 ==="
echo "===================="
echo "=== Stop OpenSplice"
$VG_OSPL_START ospl stop
sleep 2
echo "=== Start OpenSplice"
$VG_OSPL_START ospl start
$VG_START_SLEEP

#SUB_PID=`sh ./startSubscriber.sh persistent "./subResult_3.3.1.txt" &`
PUB_PID=`sh ./startPublisher.sh transient true true ./pubResult_3.1.txt`
echo "=== Publisher PID=$PUB_PID"
sh ./startSubscriber.sh transient "./subResult_3.1.txt" 
echo "=== End of DurabilityDataSubscriber (Scenario 3.1)"
#sleep 5
echo === killing publisher
kill -9 $PUB_PID

echo "===================="
echo "=== Scenario 3.2 ==="
echo "===================="
echo "=== Stop OpenSplice"
$VG_OSPL_START ospl stop
sleep 2
echo "=== Start OpenSplice"
$VG_OSPL_START ospl start
$VG_START_SLEEP

PUB_PID=`sh ./startPublisher.sh transient false true ./pubResult_3.2.txt`
echo "=== Publisher PID=$PUB_PID"
sleep 5
echo === killing publisher
kill -9 $PUB_PID
#killall DurabilityDataPublisher

echo === running a first Subscriber   > subResult_3.2.1.txt
sh ./startSubscriber.sh transient "./subResult_3.2.1.txt" 
echo "=== End of first DurabilityDataSubscriber  (Scenario 3.2)"

echo === running a second Subscriber  > subResult_3.2.2.txt
sh ./startSubscriber.sh transient "./subResult_3.2.2.txt"
echo "=== End of second DurabilityDataSubscriber (Scenario 3.2)"
sleep 2

echo "===================="
echo "=== Scenario 3.3 ==="
echo "===================="
echo "=== Stop OpenSplice"
$VG_OSPL_START ospl stop
sleep 2
echo "=== Start OpenSplice"
$VG_OSPL_START ospl start
$VG_START_SLEEP
PUB_PID=`sh ./startPublisher.sh persistent false true ./pubResult_3.3.txt`
echo "=== Publisher PID=$PUB_PID"
sleep 5

echo === running a first Subscriber
sh ./startSubscriber.sh persistent "./subResult_3.3.1.txt"
echo === "End of first DurabilityDataSubscriber (Scenario 3.3)"
sleep 2
echo === killing publisher
kill -9 $PUB_PID
echo "=== Stop OpenSplice"
$VG_OSPL_START ospl stop
sleep 2
echo "=== Start OpenSplice"
$VG_OSPL_START ospl start
$VG_START_SLEEP
echo === running a second Subscriber after stop/start of OpenSplice
sh ./startSubscriber.sh persistent "./subResult_3.3.2.txt" 
echo "=== End of second DurabilityDataSubscriber (Scenario 3.3)"
sleep 2
