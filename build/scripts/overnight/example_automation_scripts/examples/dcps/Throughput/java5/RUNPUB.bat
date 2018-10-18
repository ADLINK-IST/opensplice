start "" /B java -jar pub/java5_Throughput_pub.jar 1 0 1 15 > pubResult.txt 2>&1
echo %errorlevel% > pubReturn.txt 2>&1
