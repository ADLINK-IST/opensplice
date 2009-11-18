@ECHO OFF
ECHO Step 1: Set TAO environment 
ECHO "TAO_ROOT must be setup early" 
ECHO "Setting environment ...." 
call setenv.bat

ECHO Step 2: Set Microsoft Visual Studio Environment using VS supplied batch file 

IF NOT "%VS80COMNTOOLS%"=="" call "%VS80COMNTOOLS%"\vsvars32.bat

IF "%VS80COMNTOOLS%"=="" call "%VS90COMNTOOLS%"\vsvars32.bat

cd "%OSPL_HOME%"

ECHO Step 3: Set OSPL runtime environment 
call release.bat
ECHO Step 4: Change to the examples directory 

cd "%OSPL_HOME%"/examples

ECHO Step 5: Build examples 

IF "%VS80COMNTOOLS%"=="" devenv examples.sln /upgrade

devenv /useenv examples.sln /Build Release
ECHO Step 7: Build Java examples 
cd "%OSPL_HOME%/examples/dcps/standalone/Java/PingPong"
call BUILD.bat
cd "%OSPL_HOME%/examples/dcps/standalone/Java/Tutorial"
call BUILD.bat
cd "%OSPL_HOME%/examples/dcps/CORBA/Java/JacORB/PingPong"
call BUILD.bat
cd "%OSPL_HOME%/examples/dcps/CORBA/Java/JacORB/Tutorial"
call BUILD.bat

cd "%OSPL_HOME%/examples"

ECHO Step 8: Check for DLRL directory

IF NOT EXIST "%OSPL_HOME%/examples/dlrl/" GOTO NODLRL

ECHO Step 9: Build dlrl examples 
REM Excluded because we haven't BUILD.bat
REM cd "%OSPL_HOME%/examples/dlrl/standalone/C++/Tutorial"
REM call BUILD.bat

cd "%OSPL_HOME%/examples/dlrl/standalone/Java/Tutorial"
call BUILD.bat

:NODLRL

cd "%OSPL_HOME%/examples"

