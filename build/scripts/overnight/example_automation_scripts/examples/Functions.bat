
echo "input param is %1"

call %1
goto:EOF

:check_osplhome
   echo "OSPL_HOME is %OSPL_HOME%"
   echo "OSPL_URI is %OSPL_URI%"
   goto:EOF

:startOSPL
   IF "%EXRUNTYPE%"=="shm" (
      echo "Starting OSPL"
      ospl start
      %SLEEP5% > NUL
   )
   goto:EOF

:stopOSPL
   IF "%EXRUNTYPE%"=="shm" (
      %SLEEP5% > NUL
      echo "Stopping OSPL"
      ospl stop
      %SLEEP5% > NUL
   )
   goto:EOF

:runZero
   echo "===== calling %NAME% for %EXAMPLE_LANG% ====="
   call :stopOSPL
   call :startOSPL
   echo "=== Launching %NAME% %EXAMPLE_LANG% "
   start CMD /C RUNSUB.bat
   %SLEEP2% >NUL
   call publisher.exe %PUB_PARAMS% > pubResult.txt 2>&1
   set PUB_RESULT=%errorlevel%
   %SLEEP10% >NUL
   set /p SUB_RESULT=<subReturn.txt
   del subReturn.txt
   %SLEEP5% >NUL
   goto:EOF

:runZeroThroughput
   echo "===== calling Throughput for %EXAMPLE_LANG% ====="
   call :stopOSPL
   call :startOSPL
   echo "=== Launching Throughput %EXAMPLE_LANG% "
   start CMD /C RUNPUB.bat
   %SLEEP5% >NUL
   call subscriber.exe 10 > subResult.txt 2>&1
   set SUB_RESULT=%errorlevel%
   %SLEEP10% >NUL
   set /p PUB_RESULT=<pubReturn.txt
   del pubReturn.txt
   %SLEEP5% >NUL
   goto:EOF

:runZeroRoundTrip
   echo "===== calling RoundTrip for %EXAMPLE_LANG% ====="
   call :stopOSPL
   call :startOSPL
   echo "=== Launching RoundTrip %EXAMPLE_LANG% "
   start CMD /C RUNSUB.bat
   %SLEEP10% >NUL
   call ping.exe 100 0 10 > pingResult.txt 2>&1
   set PUB_RESULT=%errorlevel%
   %SLEEP5% >NUL
   call ping.exe quit >NUL
   %SLEEP10% >NUL
   set /p SUB_RESULT=<subReturn.txt
   del subReturn.txt
   %SLEEP5% >NUL
   goto:EOF

:checkResultZero
   echo "=== Checking %NAME% results"
   set SUCCESS=0
   if NOT %SUB_RESULT% == 0 set SUCCESS=1
   if NOT %PUB_RESULT% == 0 set SUCCESS=1
   if %SUCCESS% == 1 (
      echo NOK
      echo "*** ERROR : example %NAME% failed "
   ) else echo OK
   %SLEEP2% > NUL
   call :stopOSPL
   %SLEEP2% >NUL
   goto:EOF

:runBuiltInTopics
   call :stopOSPL
   set EXPECTED_RESULT=..\..\expected_results
   set WIN_BATCH="..\..\..\..\win_batch"
   call :startOSPL
   echo "=== Starting Subscriber %EXAMPLE_LANG% "
   start CMD /C %EXAMPLE_LANG%_builtintopics_sub.exe ^> subResult.txt
   goto:EOF

:runBuiltInTopicsISOCPP
   call :stopOSPL
   set EXPECTED_RESULT=..\expected_results
   set WIN_BATCH="..\..\..\win_batch"
   call :startOSPL
   echo "=== Starting Subscriber %EXAMPLE_LANG% "
   start CMD /C subscriber.exe ^> subResult.txt
   goto:EOF

:builtintopicsCheckResult
   %SLEEP10% > NUL
   echo "=== Checking BuiltInTopics Subscriber results"
   findstr /I "Host name for node" subResult.txt  > nul
   if "%errorlevel%"=="0" (
     echo  BuiltInTopics example - OK
   ) else (
     echo BuiltInTopics example - FAILED
   )
   %SLEEP10% > NUL
   call :stopOSPL
   goto:EOF

:runContentFilteredTopic
   call :stopOSPL
   call :startOSPL
   echo "=== Launching ContentFilteredTopic %EXAMPLE_LANG%"
   start CMD /C %EXAMPLE_LANG%_contentfilteredtopic_sub.exe GE ^> subResult.txt
   %SLEEP5% >NUL
   call %EXAMPLE_LANG%_contentfilteredtopic_pub.exe > pubResult.txt
   %SLEEP5% >NUL
   goto:EOF

:contentfilteredtopicCheckResult
   echo "=== Checking ContentFilteredTopic Subscriber results"
   findstr /I "GE" subResult.txt  > nul
   if "%errorlevel%"=="0" (
     echo  ContentFilteredTopic example - OK
   ) else (
     echo ContentFilteredTopic example - FAILED
   )
   findstr /I "MSFT" subResult.txt  > nul
   if "%errorlevel%"=="1" (
     echo  ContentFilteredTopic example - OK
   ) else (
     echo ContentFilteredTopic example - FAILED
   )
   %SLEEP2% > NUL
   call :stopOSPL
   %SLEEP2% > NUL
   goto:EOF

:runHelloWorld
   call :stopOSPL
   call :startOSPL
   echo "=== Launching HelloWorld %EXAMPLE_LANG% "
   start CMD /C %EXAMPLE_LANG%_helloworld_sub.exe ^> subResult.txt
   %SLEEP2% >NUL
   call %EXAMPLE_LANG%_helloworld_pub.exe > pubResult.txt
   %SLEEP5% >NUL
   goto:EOF

:helloworldCheckResult
   set EXPECTED_RESULT="..\..\expected_results"
   set WIN_BATCH="..\..\..\..\win_batch"
   echo "=== Checking HelloWorld Subscriber results"
   echo "=== EXPECTED_RESULT ==="
   type %EXPECTED_RESULT%\subResult.txt
   call %WIN_BATCH%\diff.bat subResult.txt %EXPECTED_RESULT%\subResult.txt
   %SLEEP2% > NUL
   call :stopOSPL
   %SLEEP2% >NUL
   goto:EOF

:runLifecycle
   call :stopOSPL
   call :startOSPL
   echo "=== Launching Lifecycle "
   echo "=== (step 1)"
   start CMD /C %EXAMPLE_LANG%_lifecycle_sub ^> subResult_1.txt
   %SLEEP2% > NUL
   call %EXAMPLE_LANG%_lifecycle_pub false dispose > pubResult_1.txt
   %SLEEP5% > NUL
   call :stopOSPL
   call :startOSPL
   echo "=== (step 2)"
   start CMD /C %EXAMPLE_LANG%_lifecycle_sub.exe ^> subResult_2.txt
   %SLEEP2% > NUL
   call %EXAMPLE_LANG%_lifecycle_pub.exe false unregister > pubResult_2.txt
   %SLEEP5% > NUL
   call :stopOSPL
   %SLEEP5% > NUL
   call :startOSPL
   echo "=== (step 3)"
   start CMD /C %EXAMPLE_LANG%_lifecycle_sub ^> subResult_3.txt
   %SLEEP2% > NUL
   call %EXAMPLE_LANG%_lifecycle_pub false unregister > pubResult_3.txt
   %SLEEP5% > NUL
   goto:EOF

:runLifecycleISOCPP
   call :stopOSPL
   call :startOSPL
   echo "=== Launching Lifecycle "
   echo "=== (step 1)"
   start CMD /C subscriber ^> subResult_1.txt
   %SLEEP2% > NUL
   call publisher false dispose > pubResult_1.txt
   %SLEEP5% > NUL
   call :stopOSPL
   call :startOSPL
   echo "=== (step 2)"
   start CMD /C subscriber.exe ^> subResult_2.txt
   %SLEEP2% > NUL
   call publisher.exe false unregister > pubResult_2.txt
   %SLEEP5% > NUL
   call :stopOSPL
   %SLEEP5% > NUL
   call :startOSPL
   echo "=== (step 3)"
   start CMD /C subscriber ^> subResult_3.txt
   %SLEEP2% > NUL
   call publisher false unregister > pubResult_3.txt
   %SLEEP5% > NUL
   goto:EOF

:lifecycleCheckResults
   echo "=== (step_1) Checking Lifecycle Subscriber results "
   findstr /I "sample_state:NOT_READ_SAMPLE_STATE-view_state:NEW_VIEW_STATE-instance_state:ALIVE_INSTANCE_STATE" subResult_1.txt  > nul
   if "%errorlevel%"=="0" (
     echo "step_1_1 - OK"
   ) else (
     echo "step_1_1 - FAILED"
   )
   findstr /I "sample_state:READ_SAMPLE_STATE-view_state:NOT_NEW_VIEW_STATE-instance_state:ALIVE_INSTANCE_STATE" subResult_1.txt  > nul
   if "%errorlevel%"=="0" (
     echo "step_1_2 - OK"
   ) else (
     echo "step_1_2 - FAILED"
   )
   findstr /I "sample_state:READ_SAMPLE_STATE-view_state:NOT_NEW_VIEW_STATE-instance_state:NOT_ALIVE_DISPOSED_INSTANCE_STATE" subResult_1.txt  > nul
   if "%errorlevel%"=="0" (
     echo "step_1_3 - OK"
   ) else (
     echo "step_1_3 - FAILED"
   )
   echo "=== (step_2) Checking Lifecycle Subscriber results "
   findstr /I "sample_state:NOT_READ_SAMPLE_STATE-view_state:NEW_VIEW_STATE-instance_state:ALIVE_INSTANCE_STATE" subResult_2.txt  > nul
   if "%errorlevel%"=="0" (
     echo "step_2_1 - OK"
   ) else (
     echo "step_2_1 - FAILED"
   )
   findstr /I "sample_state:READ_SAMPLE_STATE-view_state:NOT_NEW_VIEW_STATE-instance_state:ALIVE_INSTANCE_STATE" subResult_2.txt  > nul
   if "%errorlevel%"=="0" (
     echo "step_2_2 - OK"
   ) else (
     echo "step_2_2 - FAILED"
   )
   findstr /I "sample_state:READ_SAMPLE_STATE-view_state:NOT_NEW_VIEW_STATE-instance_state:NOT_ALIVE_NO_WRITERS_INSTANCE_STATE" subResult_2.txt  > nul
   if "%errorlevel%"=="0" (
     echo "step_2_3 - OK"
   ) else (
     echo "step_2_3 - FAILED"
   )
   echo "=== (step_3) Checking Lifecycle Subscriber results "
   findstr /I "sample_state:NOT_READ_SAMPLE_STATE-view_state:NEW_VIEW_STATE-instance_state:ALIVE_INSTANCE_STATE" subResult_3.txt  > nul
   if "%errorlevel%"=="0" (
     echo "step_3_1 - OK"
   ) else (
     echo "step_3_1 - FAILED"
   )
   findstr /I "sample_state:READ_SAMPLE_STATE-view_state:NOT_NEW_VIEW_STATE-instance_state:ALIVE_INSTANCE_STATE" subResult_3.txt  > nul
   if "%errorlevel%"=="0" (
     echo "step_3_2 - OK"
   ) else (
     echo "step_3_2 - FAILED"
   )
   findstr /I "sample_state:READ_SAMPLE_STATE-view_state:NOT_NEW_VIEW_STATE-instance_state:NOT_ALIVE_NO_WRITERS_INSTANCE_STATE" subResult_3.txt  > nul
   if "%errorlevel%"=="0" (
     echo "step_3_3 - OK"
   ) else (
     echo "step_3_3 - FAILED"
   )
   call :stopOSPL
   goto:EOF

:runListener
   call :stopOSPL
   call :startOSPL
   echo "=== Launching Listener %EXAMPLE_LANG%"
   start CMD /C %EXAMPLE_LANG%_listener_sub.exe ^> subResult.txt
   %SLEEP2% > NUL
   call %EXAMPLE_LANG%_listener_pub.exe > pubResult.txt
   goto:EOF

:listenerCheckResults
   %SLEEP5% >NUL
   echo "=== Checking  Listener results"
   findstr /I "message received" subResult.txt  > nul
   if "%errorlevel%"=="0" (
     echo "Listener example - OK (1)"
   ) else (
     echo "Listener example - FAILED (1)"
   )
   findstr /I "userID" subResult.txt  > nul
   if "%errorlevel%"=="0" (
     echo "Listener example - OK (2)"
   ) else (
     echo "Listener example - FAILED (2)"
   )
   findstr /I "Message :" subResult.txt  > nul
   if "%errorlevel%"=="0" (
     echo "Listener example - OK (3)"
   ) else (
     echo "Listener example - FAILED (3)"
   )
   call :stopOSPL
   goto:EOF

:runOwnership
   call :stopOSPL
   call :startOSPL
   echo "=== Launching Ownership %EXAMPLE_LANG%"
   start CMD /C %EXAMPLE_LANG%_ownership_sub.exe ^> subResult.txt
   %SLEEP2% >NUL
   echo "=== starting publisher pub1 with ownership strength 5"
   start CMD /C %EXAMPLE_LANG%_ownership_pub.exe "pub1" 5 40 1  ^> pub1Result.txt
   echo "=== Waiting 2 seconds ..."
   %SLEEP2% >NUL
   echo "=== starting publisher pub2 with ownership strength 10"
   start CMD /C %EXAMPLE_LANG%_ownership_pub.exe "pub2" 10 5 0 ^> pub2Result.txt
   goto:EOF

:runOwnershipISOCPP
   call :stopOSPL
   call :startOSPL
   echo "=== Launching Ownership %EXAMPLE_LANG%"
   start CMD /C subscriber.exe ^> subResult.txt
   %SLEEP2% >NUL
   echo "=== starting publisher pub1 with ownership strength 5"
   start CMD /C publisher.exe "pub1" 5 40 true  ^> pub1Result.txt
   echo "=== Waiting 2 seconds ..."
   %SLEEP2% >NUL
   echo "=== starting publisher pub2 with ownership strength 10"
   start CMD /C publisher.exe "pub2" 10 5 false ^> pub2Result.txt
   goto:EOF

:ownershipCheckResults
   %SLEEP5% > NUL
   findstr /I "pub1" subResult.txt  > nul
   if "%errorlevel%"=="0" (
     echo "Ownership example - OK (1)"
   ) else (
     echo "Ownership example - FAILED (1)"
   )
   findstr /I "pub2" subResult.txt  > nul
   if "%errorlevel%"=="0" (
     echo "Ownership example - OK (2)"
   ) else (
     echo "Ownership example - FAILED (2)"
   )
   call :stopOSPL
   goto:EOF

:runQueryCondition
   call :stopOSPL
   call :startOSPL
   echo "=== Launching QueryCondition %EXAMPLE_LANG%"
   start CMD /C %EXAMPLE_LANG%_querycondition_sub.exe MSFT ^> subResult.txt
   %SLEEP2% >NUL
   call %EXAMPLE_LANG%_querycondition_pub.exe > pubResult.txt
   goto:EOF

:queryconditionCheckResults
   %SLEEP5% >NUL
   echo "=== Checking QueryCondition Subscriber results"
   findstr /I "MSFT" subResult.txt  > nul
   if "%errorlevel%"=="0" (
     echo  "QueryCondition example - OK (1)"
   ) else (
     echo "QueryCondition example - FAILED (1)"
   )
   findstr /I "GE:" subResult.txt  > nul
   if "%errorlevel%"=="1" (
     echo  "QueryCondition example - OK (2)"
   ) else (
     echo "QueryCondition example - FAILED (2)"
   )
   call :stopOSPL
   goto:EOF

:runWaitSet
   call :stopOSPL
   call :startOSPL
   echo "=== Launching WaitSet %EXAMPLE_LANG%"
   start CMD /C %EXAMPLE_LANG%_waitset_sub.exe ^> subResult.txt
   %SLEEP2% > NUL
   call %EXAMPLE_LANG%_waitset_pub.exe > pubResult.txt
   goto:EOF

:waitsetCheckResults
   %SLEEP5% >NUL
   echo "Checking results"
   findstr /I "a MsgWriter joined" subResult.txt  > nul
   if "%errorlevel%"=="0" (
     echo  "WaitSet example - OK (1)"
   ) else (
     echo "WaitSet example - FAILED (1)"
   )
   findstr /I "First Hello" subResult.txt  > nul
   if "%errorlevel%"=="0" (
     echo  "WaitSet example - OK (2)"
   ) else (
     echo "WaitSet example - FAILED (2)"
   )
   findstr /I "Hello again" subResult.txt  > nul
   if "%errorlevel%"=="0" (
     echo  "WaitSet example - OK (3)"
   ) else (
     echo "WaitSet example - FAILED (3)"
   )
   findstr /I "a MsgWriter lost its liveliness" subResult.txt  > nul
   if "%errorlevel%"=="0" (
     echo  "WaitSet example - OK (4)"
   ) else (
     echo "WaitSet example - FAILED (4)"
   )
   findstr /I "escape condition triggered" subResult.txt  > nul
   if "%errorlevel%"=="0" (
     echo  "WaitSet example - OK (5)"
   ) else (
     echo "WaitSet example - FAILED (5)"
   )
   call :stopOSPL
   goto:EOF

:runDurabilityInit
   if "%EXRUNTYPE%"=="shm" (
      set OSPL_URI=file://%OSPL_HOME%\examples\dcps\Durability\ospl_shm.xml
   ) else (
      set OSPL_URI=file://%OSPL_HOME%\examples\dcps\Durability\ospl_sp.xml
   )
   echo "OSPL_URI is %OSPL_URI%"
   call :stopOSPL
   call :startOSPL
   goto:EOF

:runDurability
   echo "=== Launching Durability "
   echo "=== Scenario 3.1"
   echo "=== running the Subscriber (%EXAMPLE_LANG%)"
   start CMD /C "%EXAMPLE_LANG%_durability_sub" transient ^> subResult_3_1.txt
   %SLEEP5% > NUL
   echo "=== running the Publisher"
   start CMD /C "%EXAMPLE_LANG%_durability_pub" transient true true ^> pubResult_3_1.txt
   echo "Wait 30s to allow the publisher to complete and terminate rather than kill it...."
   %SLEEP30% > NUL
   call :stopOSPL
   call :startOSPL
   %SLEEP5% >NUL
   echo "=== Scenario 3.2"
   echo "=== running a first Subscriber %EXAMPLE_LANG%_durability_sub"
   start CMD /C %EXAMPLE_LANG%_durability_sub transient ^> subResult_3_2_1.txt
   echo "=== running a second Subscriber"
   start CMD /C %EXAMPLE_LANG%_durability_sub transient ^> subResult_3_2_2.txt
   %SLEEP5% >NUL
   echo "=== running the Publisher"
   start CMD /C %EXAMPLE_LANG%_durability_pub transient false true ^> pubResult_3_2.txt
   echo "Wait 30s to allow the publisher to complete and terminate rather than kill it"
   %SLEEP30% > NUL
   call :stopOSPL
   echo "=== Scenario 3.3"
   echo "=== Stop OpenSplice"
   call :startOSPL
   echo "=== running a first Subscriber"
   start CMD /C %EXAMPLE_LANG%_durability_sub persistent ^> subResult_3_3_1.txt
   %SLEEP5% >NUL
   echo "=== running the Publisher"
   start CMD /C %EXAMPLE_LANG%_durability_pub persistent false true ^> pubResult_3_3.txt
   echo "Wait 30s to allow the publisher to complete and terminate rather than kill it"
   %SLEEP30% > NUL
   call :stopOSPL
   call :startOSPL
   echo "=== running a second Subscriber after stop/start of OpenSplice"
   start CMD /C %EXAMPLE_LANG%_durability_sub persistent ^> subResult_3_3_2.txt
   %SLEEP5% >NUL
   goto:EOF

:durabilityCheckResults
   echo "================== Scenario 3.1 ========================"
   echo "=== Checking Durability Subscriber results Scenario 3.1"
   tail --lines=10 subResult_3_1.txt > tail_subResult.txt
   diff -B --strip-trailing-cr tail_subResult.txt ..\..\expected_result > NUL
   if "%errorlevel%"=="0" (
     echo "Scenario 3.1 - OK "
   ) else (
     echo "Scenario 3.1 - FAILED"
   )
   %SLEEP5% > NUL
   echo "================== Scenario 3.2 ========================"
   echo "=== Checking Durability second Subscriber results Scenario 3.2"
   echo "Checking only result of second subscriber ..\..\expected_result"
   tail --lines=10 subResult_3_2_2.txt > tail_subResult.txt
   diff -B --strip-trailing-cr tail_subResult.txt ..\..\expected_result > NUL
   if "%errorlevel%"=="0" (
     echo "Scenario 3.2 - OK "
   ) else (
     echo "Scenario 3.2 - FAILED"
   )
   %SLEEP5% > NUL
   echo "================== Scenario 3.3 ========================"
   echo "=== Checking Durability second Subscriber results Scenario 3.3"
   echo "    (not empty after restarting OpenSplice)"
   echo "checking only that result of second subscriber is not empty"
   tail --lines=10 subResult_3_3_2.txt > tail_subResult.txt
   diff -B --strip-trailing-cr tail_subResult.txt ..\..\expected_result > NUL
   if "%errorlevel%"=="0" (
     echo "Scenario 3.3 - OK "
   ) else (
     echo "Scenario 3.3 - FAILED"
   )
   call :stopOSPL
   goto:EOF

:runDurabilityISOCPP
   echo "=== Launching Durability "
   echo "=== Scenario 3.1"
   echo "=== running the Subscriber (%EXAMPLE_LANG%)"
   start CMD /C "subscriber" transient ^> subResult_3_1.txt
   %SLEEP5% > NUL
   echo "=== running the Publisher"
   start CMD /C "publisher" transient true true ^> pubResult_3_1.txt
   echo "Wait 30s to allow the publisher to complete and terminate rather than kill it...."
   %SLEEP30% > NUL
   call :stopOSPL
   call :startOSPL
   %SLEEP5% >NUL
   echo "=== Scenario 3.2"
   echo "=== running a first Subscriber subscriber"
   start CMD /C subscriber transient ^> subResult_3_2_1.txt
   echo "=== running a second Subscriber"
   start CMD /C subscriber transient ^> subResult_3_2_2.txt
   %SLEEP5% >NUL
   echo "=== running the Publisher"
   start CMD /C publisher transient false true ^> pubResult_3_2.txt
   echo "Wait 30s to allow the publisher to complete and terminate rather than kill it"
   %SLEEP30% > NUL
   call :stopOSPL
   echo "=== Scenario 3.3"
   echo "=== Stop OpenSplice"
   call :startOSPL
   echo "=== running a first Subscriber"
   start CMD /C subscriber persistent ^> subResult_3_3_1.txt
   %SLEEP5% >NUL
   echo "=== running the Publisher"
   start CMD /C publisher persistent false true ^> pubResult_3_3.txt
   echo "Wait 30s to allow the publisher to complete and terminate rather than kill it"
   %SLEEP30% > NUL
   call :stopOSPL
   call :startOSPL
   echo "=== running a second Subscriber after stop/start of OpenSplice"
   start CMD /C subscriber persistent ^> subResult_3_3_2.txt
   %SLEEP5% >NUL
   goto:EOF

:durabilityCheckResultsISOCPP
   echo "================== Scenario 3.1 ========================"
   echo "=== Checking Durability Subscriber results Scenario 3.1"
   tail --lines=10 subResult_3_1.txt > tail_subResult.txt
   diff -B --strip-trailing-cr tail_subResult.txt ..\..\expected_result > NUL
   if "%errorlevel%"=="0" (
     echo "Scenario 3.1 - OK "
   ) else (
     echo "Scenario 3.1 - FAILED"
   )
   %SLEEP5% > NUL
   echo "================== Scenario 3.2 ========================"
   echo "=== Checking Durability second Subscriber results Scenario 3.2"
   echo "Checking only result of second subscriber ..\..\expected_result"
   tail --lines=10 subResult_3_2_2.txt > tail_subResult.txt
   diff -B --strip-trailing-cr tail_subResult.txt ..\..\expected_result > NUL
   if "%errorlevel%"=="0" (
     echo "Scenario 3.2 - OK "
   ) else (
     echo "Scenario 3.2 - FAILED"
   )
   %SLEEP5% > NUL
   echo "================== Scenario 3.3 ========================"
   echo "=== Checking Durability second Subscriber results Scenario 3.3"
   echo "    (not empty after restarting OpenSplice)"
   echo "checking only that result of second subscriber is not empty"
   tail --lines=10 subResult_3_3_2.txt > tail_subResult.txt
   diff -B --strip-trailing-cr tail_subResult.txt ..\..\expected_result > NUL
   if "%errorlevel%"=="0" (
     echo "Scenario 3.3 - OK "
   ) else (
     echo "Scenario 3.3 - FAILED"
   )
   call :stopOSPL
   goto:EOF

:runPingPong
   if not exist "pong.exe" (
   echo "Error: The pong executable does not exist - aborting example" >> %LOGFILE%
   call :stopOSPL
   goto:EOF
   )
   if not exist "ping.exe" (
   echo "Error: The ping executable does not exist - aborting example" >> %LOGFILE%
   call :stopOSPL
   goto:EOF
   )
   set BLOKSIZE=100
   set BLOKCOUNT=100
   call :check_osplhome
   call :startOSPL
   ECHO "Starting Pong"
   set LEVEL=Starting pong
   start CMD /C pong.exe PongRead PongWrite ^> pong.log
   if %ERRORLEVEL% NEQ 0 GOTO PingPongErr
   %SLEEP4% >NUL
   ECHO "Starting ping with m"
   set LEVEL=Starting ping with m
   ping.exe %BLOKCOUNT% %BLOKSIZE% m PongRead PongWrite >> %LOGFILE%
   if %ERRORLEVEL% NEQ 0 GOTO PingPongErr
   %SLEEP4% >NUL
   ECHO "Starting ping with q"
   set LEVEL=Starting ping with q
   ping.exe %BLOKCOUNT% %BLOKSIZE% q PongRead PongWrite >> %LOGFILE%
   if %ERRORLEVEL% NEQ 0 GOTO PingPongErr
   %SLEEP4% >NUL
   ECHO "Starting ping with s"
   set LEVEL=Starting ping with s
   ping.exe %BLOKCOUNT% %BLOKSIZE% s PongRead PongWrite >> %LOGFILE%
   if %ERRORLEVEL% NEQ 0 GOTO PingPongErr
   %SLEEP4% >NUL
   ECHO "Starting ping with f"
   set LEVEL=Starting ping with f
   ping.exe %BLOKCOUNT% %BLOKSIZE% f PongRead PongWrite >> %LOGFILE%
   if %ERRORLEVEL% NEQ 0 GOTO PingPongErr
   %SLEEP4% >NUL
   ECHO "Starting ping with t"
   set LEVEL=Starting ping with t
   ping.exe 1  10 t PongRead PongWrite >> %LOGFILE%
   if %ERRORLEVEL% NEQ 0 GOTO PingPongErr
   call :stopOSPL
   goto:EOF

:PingPongErr
   ECHO "An error occurred %LEVEL%  ....." >> %LOGFILE%
   goto:EOF

:runTutorial
   if not exist "MessageBoard.exe" (
   echo "Error: The MessageBoard executable does not exist - aborting example" >> %LOGFILE%
   call :stopOSPL
   goto:EOF
   )
   if not exist "Chatter.exe" (
   echo "Error: The Chatter executable does not exist - aborting example" >> %LOGFILE%
   call :stopOSPL
   goto:EOF
   )
   call :stopOSPL
   call :startOSPL
   ECHO "Starting MessageBoard"
   SET LEVEL=Starting MessageBoard
   start CMD /C MessageBoard.exe ^> MessageBoard.log
   if %ERRORLEVEL% NEQ 0 GOTO TutorialErr
   %SLEEP4% >NUL
   if exist "UserLoad.exe" (
      ECHO "Starting UserLoad"
      SET LEVEL=Starting UserLoad
      start CMD /C UserLoad.exe ^> UserLoad.log
      if %ERRORLEVEL% NEQ 0 "ECHO An error occurred starting UserLoad %ERRORLEVEL%"
   )else (
      echo "Error: The UserLoad executable does not exist" >> %LOGFILE%
   )
   %SLEEP4% >NUL
   ECHO "Starting Chatter"
   SET LEVEL=Staring Chatter
   Chatter.exe >> %LOGFILE%
   if %ERRORLEVEL% NEQ 0 GOTO TutorialErr
   %SLEEP4% >NUL
   ECHO "Starting Chatter with terminate message"
   SET LEVEL=Starting Chatter with -1
   Chatter.exe -1 >> %LOGFILE%
   if %ERRORLEVEL% NEQ 0 GOTO TutorialErr
   %SLEEP5% > NUL
   call :stopOSPL
   goto:EOF

:TutorialErr
   ECHO "An error occurred %LEVEL%, exiting example ..." >> %LOGFILE%
   goto:EOF
