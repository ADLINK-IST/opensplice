@ECHO OFF
ECHO Set TAO environment 
ECHO "TAO_ROOT must be setup early" 
ECHO "Setting environment ...." 

cd examples 

call setenv.bat

ECHO Set Microsoft Visual Studio Environment using VS supplied batch file 

IF NOT "%VS80COMNTOOLS%"=="" call "%VS80COMNTOOLS%"\vsvars32.bat

IF "%VS80COMNTOOLS%"=="" call "%VS90COMNTOOLS%"\vsvars32.bat

cd "%OSPL_HOME%"

ECHO Set OSPL runtime environment 
call release.bat

ECHO Change to the examples directory 

cd "%OSPL_HOME%"/examples

ECHO Build examples 

IF "%VS80COMNTOOLS%"=="" devenv examples.sln /upgrade

devenv /useenv examples.sln /Build Release

ECHO Build Java examples 
ECHO Building dcps/standalone/Java/PingPong
cd "%OSPL_HOME%/examples/dcps/standalone/Java/PingPong"
call BUILD.bat

ECHO Building dcps/standalone/Java/Tutorial
cd "%OSPL_HOME%/examples/dcps/standalone/Java/Tutorial"
call BUILD.bat

ECHO Building dcps/CORBA/Java/JacORB/PingPong
cd "%OSPL_HOME%/examples/dcps/CORBA/Java/JacORB/PingPong"
call BUILD.bat

ECHO Building dcps/CORBA/Java/JacORB/Tutorial
cd "%OSPL_HOME%/examples/dcps/CORBA/Java/JacORB/Tutorial"
call BUILD.bat

cd "%OSPL_HOME%/examples"

ECHO Check for DLRL directory

IF NOT EXIST "%OSPL_HOME%/examples/dlrl/" GOTO NODLRL

ECHO Build dlrl examples 
ECHO Not building dlrl/standalone/C++/Tutorial because there is no BUILD.bat
REM cd "%OSPL_HOME%/examples/dlrl/standalone/C++/Tutorial"
REM call BUILD.bat

ECHO Building dlrl/standalone/Java/Tutorial
cd "%OSPL_HOME%/examples/dlrl/standalone/Java/Tutorial"
call BUILD.bat

GOTO END

:NODLRL
ECHO No DLRL examples to build

:END

cd "%OSPL_HOME%/examples"

