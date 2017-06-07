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

echo Building ISO_Cxx_Tutorial_Types.%projsfx%
if %use_devenv% == 1 (
   devenv "%OSPL_HOME%examples\DCPS_ISO_CPlusPlus.sln" /%1 Release /project "%OSPL_HOME%examples\dcps\Tutorial\isocpp\ISO_Cxx_Tutorial_Types.%projsfx%"
) else (
    msbuild ISO_Cxx_Tutorial_Types.%projsfx% /p:Configuration=Release
)

IF NOT %ERRORLEVEL% == 0 (
ECHO:
ECHO *** Error building 
ECHO: 
GOTO error
)
cd %~dp0

echo Building ISO_Cxx_Tutorial_Impl.%projsfx%
if %use_devenv% == 1 (
   devenv "%OSPL_HOME%examples\DCPS_ISO_CPlusPlus.sln" /%1 Release /project "%OSPL_HOME%examples\dcps\Tutorial\isocpp\ISO_Cxx_Tutorial_Impl.%projsfx%"
) else (
    msbuild ISO_Cxx_Tutorial_Impl.%projsfx% /p:Configuration=Release
)

IF NOT %ERRORLEVEL% == 0 (
ECHO:
ECHO *** Error building 
ECHO: 
GOTO error
)
cd %~dp0

echo Building ISO_Cxx_Tutorial_Chatter.%projsfx%
if %use_devenv% == 1 (
   devenv "%OSPL_HOME%examples\DCPS_ISO_CPlusPlus.sln" /%1 Release /project "%OSPL_HOME%examples\dcps\Tutorial\isocpp\ISO_Cxx_Tutorial_Chatter.%projsfx%"
) else (
    msbuild ISO_Cxx_Tutorial_Chatter.%projsfx% /p:Configuration=Release
)
IF NOT %ERRORLEVEL% == 0 (
ECHO:
ECHO *** Error building 
ECHO: 
GOTO error
)
cd %~dp0

echo Building ISO_Cxx_Tutorial_MessageBoard.%projsfx%
if %use_devenv% == 1 (
   devenv "%OSPL_HOME%examples\DCPS_ISO_CPlusPlus.sln" /%1 Release /project "%OSPL_HOME%examples\dcps\Tutorial\isocpp\ISO_Cxx_Tutorial_MessageBoard.%projsfx%"
) else (
    msbuild ISO_Cxx_Tutorial_MessageBoard.%projsfx% /p:Configuration=Release
)

IF NOT %ERRORLEVEL% == 0 (
ECHO:
ECHO *** Error building 
ECHO: 
GOTO error
)
cd %~dp0

echo Building ISO_Cxx_Tutorial_UserLoad.%projsfx%
if %use_devenv% == 1 (
   devenv "%OSPL_HOME%examples\DCPS_ISO_CPlusPlus.sln" /%1 Release /project "%OSPL_HOME%examples\dcps\Tutorial\isocpp\ISO_Cxx_Tutorial_UserLoad.%projsfx%"
) else (
    msbuild ISO_Cxx_Tutorial_UserLoad.%projsfx% /p:Configuration=Release
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
