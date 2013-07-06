@ECHO OFF
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
ECHO Cleaning Visual Studio examples....
FOR %%p in (Debug Release) DO (
    FOR %%q IN (*.sln) DO (
        devenv.com %%q /clean %%p >NUL 2>&1
    )
)
ECHO Building Visual Studio examples....
FOR %%g in (Debug Release) DO (
    ECHO Building all Visual Studio example solutions - Configuration %%g ....
    FOR %%f IN (*.sln) DO (
        ECHO -----------------------------------------------------------------------
        ECHO Building solution file %%f with configuration %%g using devenv.com
        ECHO devenv.com %%f /build %%g
        devenv.com %%f /build %%g
        IF ERRORLEVEL 1 ECHO ***** Error building %%f %%g using devenv.com Return code %ERRORLEVEL%
    )
)

ECHO Building Java examples....
FOR %%f IN (BUILD*.bat) DO (
    ECHO -----------------------------------------------------------------------
    ECHO Calling java build batch file %%f
    call %%f
    IF ERRORLEVEL 1 ECHO ***** Error calling %%f Return code %ERRORLEVEL%
    cd "%OSPL_HOME%"/examples
)

ECHO -----------------------------------------------------------------------
REM The java dlrl example is still using BUILD.bat for now
cd "%OSPL_HOME%\examples\dlrl\standalone\java\Tutorial
call BUILD.bat

ECHO ON

cd "%~dp0\..\..\..\testsuite\tests"

ECHO Building system testcases

REM IF "%VS80COMNTOOLS%"=="" devenv tests.sln /upgrade

REM devenv tests.sln /Clean %BUILD_CONFIG%
REM devenv tests.sln /Rebuild %BUILD_CONFIG%

ECHO OFF

cd "%OSPL_HOME%/examples"

