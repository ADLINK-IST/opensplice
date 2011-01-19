SET SLEEP2=ping -n 5 127.0.0.1 
echo === starting publisher "pub1" with ownership strength 5
start /B startPublisher.bat "pub1" 5 40 1	
echo === Waiting 2 seconds ...
%SLEEP2% >NUL
echo === starting publisher "pub2" with ownership strength 10
start /B  startPublisher.bat "pub2" 10 5 0 
