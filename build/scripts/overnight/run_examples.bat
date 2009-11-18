@ECHO OFF
ECHO Step 1: Set TAO environment
ECHO "TAO_ROOT must be setup early"

call setenv.bat

ECHO Step 2: Set Microsoft Visual Studio Environment using VS supplied batch file

IF NOT "%VS80COMNTOOLS%"=="" call "%VS80COMNTOOLS%"\vsvars32.bat

IF "%VS80COMNTOOLS%"=="" call "%VS90COMNTOOLS%"\vsvars32.bat

cd "%OSPL_HOME%"

ECHO Step 3: Set OSPL runtime environment
call release.bat
ECHO Step 4: Change to the examples directory

ECHO Step 5: Run C examples
cd "%OSPL_HOME%/examples/dcps/standalone/C/PingPong"
call RUN.bat
cd "%OSPL_HOME%/examples/dcps/standalone/C/Tutorial"
call RUN.bat

ECHO Step 6: Run C++ examples
cd "%OSPL_HOME%/examples/dcps/standalone/C++/PingPong"
call RUN.bat
cd "%OSPL_HOME%/examples/dcps/standalone/C++/Tutorial"
call RUN.bat

ECHO Step 7: Build Java examples
cd "%OSPL_HOME%/examples/dcps/standalone/Java/PingPong"
call RUN.bat
cd "%OSPL_HOME%/examples/dcps/standalone/Java/Tutorial"
call RUN.bat

cd "%OSPL_HOME%/examples"

ECHO Step 8: Check for DLRL directory

IF NOT EXIST "%OSPL_HOME%/examples/dlrl/" GOTO NODLRL

ECHO Step 9: Build dlrl examples
REM Excluded because we haven't BUILD.bat
REM cd "%OSPL_HOME%/examples/dlrl/standalone/C++/Tutorial"
REM call RUN.bat

cd "%OSPL_HOME%/examples/dlrl/standalone/Java/Tutorial"
call RUN.bat

:NODLRL

cd "%OSPL_HOME%/examples"
