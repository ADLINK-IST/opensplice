@echo off

echo set the Visual Studio environment by calling vcvarsall.bat
call "%VCINSTALLDIR%"/vcvarsall.bat

REM The express version of Visual Studio does not have devenv and
REM uses msbuild.  Older versions of Visual Studio still need to 
REM use devenv
set use_devenv=0

which devenv > nul 2>&1
if %ERRORLEVEL% == 0 set use_devenv=1

echo Building sacs_builtintopics_sub.csproj
if %use_devenv% == 1 (
   devenv "%OSPL_HOME%examples\CSharp.sln" /%1 Release /project "%OSPL_HOME%examples\dcps\BuiltInTopics\cs\standalone\sacs_builtintopics_sub.csproj"
) else (
   msbuild sacs_builtintopics_sub.csproj /p:Configuration=Release
)

IF NOT %ERRORLEVEL% == 0 (
ECHO:
ECHO *** Error building 
ECHO: 
GOTO error
)
cd %~dp0

GOTO end
:error
ECHO An error occurred, exiting now
:end
