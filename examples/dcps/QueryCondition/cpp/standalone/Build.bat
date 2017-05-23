@echo off
setlocal

echo set the Visual Studio environment by calling vcvarsall.bat
call "%VCINSTALLDIR%"/vcvarsall.bat  %PROCESSOR_ARCHITECTURE%

REM The express version of Visual Studio does not have devenv and
REM uses msbuild.  Older versions of Visual Studio still need to 
REM use devenv
set use_devenv=0

which devenv > nul 2>&1
if %ERRORLEVEL% == 0 set use_devenv=1

set projsfx=vcxproj
echo "%VCINSTALLDIR%" | findstr /C:"Studio 8" 1>nul
IF %ERRORLEVEL% == 0 set projsfx=vcproj

echo "%VCINSTALLDIR%" | findstr /C:"Studio 9.0" 1>nul
IF %ERRORLEVEL% == 0 set projsfx=vcproj

echo Building sacpp_querycondition_types.%projsfx%
if %use_devenv% == 1 (
   devenv "%OSPL_HOME%examples\All_Standalone_C_and_CPlusPlus.sln" /%1 Release /project "%OSPL_HOME%examples\dcps\QueryCondition\cpp\standalone\sacpp_querycondition_types.%projsfx%"
) else (
   msbuild sacpp_querycondition_types.%projsfx% /p:Configuration=Release
)

IF NOT %ERRORLEVEL% == 0 (
ECHO:
ECHO *** Error building 
ECHO: 
GOTO error
)
cd %~dp0

echo Building sacpp_querycondition_sub.%projsfx%
if %use_devenv% == 1 (
   devenv "%OSPL_HOME%examples\All_Standalone_C_and_CPlusPlus.sln" /%1 Release /project "%OSPL_HOME%examples\dcps\QueryCondition\cpp\standalone\sacpp_querycondition_sub.%projsfx%"
) else (
   msbuild sacpp_querycondition_sub.%projsfx% /p:Configuration=Release
)

IF NOT %ERRORLEVEL% == 0 (
ECHO:
ECHO *** Error building 
ECHO: 
GOTO error
)
cd %~dp0

echo Building sacpp_querycondition_pub.%projsfx%
if %use_devenv% == 1 (
   devenv "%OSPL_HOME%examples\All_Standalone_C_and_CPlusPlus.sln" /%1 Release /project "%OSPL_HOME%examples\dcps\QueryCondition\cpp\standalone\sacpp_querycondition_pub.%projsfx%"
) else (
   msbuild sacpp_querycondition_pub.%projsfx% /p:Configuration=Release
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
