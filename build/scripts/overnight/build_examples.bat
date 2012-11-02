@ECHO OFF
ECHO Set TAO environment
ECHO "TAO_ROOT must be setup early"
ECHO "Setting environment ...."

cd examples

call setenv.bat

ECHO Set Microsoft Visual Studio Environment using VS supplied batch file

IF "%VS_ENV_SCRIPT%"=="" EXIT 1

IF NOT "%VS_ENV_SCRIPT%"=="" call "%VS_ENV_SCRIPT%"

cd "%OSPL_HOME%"

ECHO Set OSPL runtime environment
call release.bat

ECHO Change to the examples directory

cd "%OSPL_HOME%"/examples

ECHO Build examples

IF "%VS80COMNTOOLS%"=="" devenv examples.sln /upgrade

devenv /useenv /Clean %BUILD_CONFIG% examples.sln
devenv /useenv /Rebuild %BUILD_CONFIG% examples.sln

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

REM  New examples - integrated since V5.4.0

echo "*** New API C/C++/C# Standalone and CORBA Examples *********************************************************"


IF "%VS80COMNTOOLS%"=="" devenv NewExamples.sln /upgrade
cd "%OSPL_HOME%"/examples

devenv /useenv /Clean %BUILD_CONFIG% NewExamples.sln
devenv /useenv /Rebuild %BUILD_CONFIG% NewExamples.sln

echo "*** New API Java examples **********************************************************************************"
echo "Example dir is: %OSPL_HOME%/examples/dcps/HelloWorld/Java/Standalone/VS2005"
cd "%OSPL_HOME%/examples/dcps/HelloWorld/Java/Standalone/Windows"
cd
call BUILD.bat

echo "Example dir is: %OSPL_HOME%/examples/dcps/HelloWorld/Java/Corba/JacORB/VS2005"
cd "%OSPL_HOME%/examples/dcps/HelloWorld/Java/Corba/JacORB/Windows"
cd
call BUILD.bat

echo "Example dir is: %OSPL_HOME%/examples/dcps/ContentFilteredTopic/Java/Standalone/VS2005"
cd "%OSPL_HOME%/examples/dcps/ContentFilteredTopic/Java/Standalone/Windows"
cd
call BUILD.bat

echo "Example dir is: %OSPL_HOME%/examples/dcps/ContentFilteredTopic/Java/Corba/JacORB/VS2005"
cd "%OSPL_HOME%/examples/dcps/ContentFilteredTopic/Java/Corba/JacORB/Windows"
cd
call BUILD.bat

echo "Example dir is: %OSPL_HOME%/examples/dcps/Durability/Java/Standalone/Windows"
cd "%OSPL_HOME%/examples/dcps/Durability/Java/Standalone/Windows"
cd
call BUILD.bat

echo "Example dir is: %OSPL_HOME%/examples/dcps/Durability/Java/Corba/JacORB/VS2005"
cd "%OSPL_HOME%/examples/dcps/Durability/Java/Corba/JacORB/Windows"
cd
call BUILD.bat

echo "Example dir is: %OSPL_HOME%/examples/dcps/Listener/Java/Standalone/VS2005"
cd "%OSPL_HOME%/examples/dcps/Listener/Java/Standalone/Windows"
cd
call BUILD.bat

echo "Example dir is: %OSPL_HOME%/examples/dcps/Listener/Java/Corba/JacORB/VS2005"
cd "%OSPL_HOME%/examples/dcps/Listener/Java/Corba/JacORB/Windows"
cd
call BUILD.bat

echo "Example dir is: %OSPL_HOME%/examples/dcps/Ownership/Java/Standalone/VS2005"
cd "%OSPL_HOME%/examples/dcps/Ownership/Java/Standalone/Windows"
cd
call BUILD.bat

cd "%OSPL_HOME%/examples/dcps/Ownership/Java/Corba/JacORB/Windows"
cd
call BUILD.bat

echo "Example dir is: %OSPL_HOME%/examples/dcps/QueryCondition/Java/Standalone/VS2005"
cd "%OSPL_HOME%/examples/dcps/QueryCondition/Java/Standalone/Windows"
cd
call BUILD.bat

echo "Example dir is: %OSPL_HOME%/examples/dcps/QueryCondition/Java/Corba/JacORB/VS2005"
cd "%OSPL_HOME%/examples/dcps/QueryCondition/Java/Corba/JacORB/Windows"
cd
call BUILD.bat

echo "Example dir is: %OSPL_HOME%/examples/dcps/WaitSet/Java/Standalone/VS2005"
cd "%OSPL_HOME%/examples/dcps/WaitSet/Java/Standalone/Windows"
cd
call BUILD.bat

echo "Example dir is: %OSPL_HOME%/examples/dcps/WaitSet/Java/Corba/JacORB/VS2005"
cd "%OSPL_HOME%/examples/dcps/WaitSet/Java/Corba/JacORB/Windows"
cd
call BUILD.bat

echo "Example dir is: %OSPL_HOME%/examples/dcps/BuiltInTopics/Java/Standalone/VS2005"
cd "%OSPL_HOME%/examples/dcps/BuiltInTopics/Java/Standalone/Windows"
cd
call BUILD.bat

echo "Example dir is: %OSPL_HOME%/examples/dcps/BuiltInTopics/Java/Corba/JacORB/VS2005"
cd "%OSPL_HOME%/examples/dcps/BuiltInTopics/Java/Corba/JacORB/Windows"
cd
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

ECHO ON

cd "%~dp0\..\..\..\testsuite\tests"

ECHO Building system testcases

REM IF "%VS80COMNTOOLS%"=="" devenv tests.sln /upgrade

REM devenv /useenv tests.sln /Clean %BUILD_CONFIG%
REM devenv /useenv tests.sln /Rebuild %BUILD_CONFIG%

ECHO OFF

cd "%OSPL_HOME%/examples"

