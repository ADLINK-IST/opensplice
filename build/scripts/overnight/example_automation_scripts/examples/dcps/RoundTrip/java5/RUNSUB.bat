start "" /B java -jar pong/java5_pong.jar %SUB_PARAMS% > pongResult.txt 2>&1
echo %errorlevel% > subReturn.txt 2>&1
