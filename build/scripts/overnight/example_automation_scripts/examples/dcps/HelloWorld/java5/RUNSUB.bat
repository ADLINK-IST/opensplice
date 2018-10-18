start "" /B  java -jar sub/java5_HelloWorld_sub.jar > subResult.txt
echo %errorlevel% > subReturn.txt 2>&1
