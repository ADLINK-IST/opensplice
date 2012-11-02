start startSubscriber.bat SubResult.txt
rem 2 seconds
set wait=2
ping 127.0.0.1 -n %wait% > NUL
start startPublisher.bat PubResult.txt
set wait=2
ping 127.0.0.1 -n %wait% > NUL
echo === checking results
echo n | comp ..\Release\PubResult.txt ..\Bat\expectedPubResult.txt
echo n | comp ..\Release\SubResult.txt ..\Bat\expectedSubResult.txt