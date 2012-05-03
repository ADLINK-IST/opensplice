@echo OFF
pushd %~dp0
rem usage : run_all_standalone.bat examples_list language
rem language = C++ | C  | Java
if "%2" == "" (
  echo "*** usage : ./run_all_standalone.sh examples_list language"
)
SET LIST=%1
SET LANG=%2
SET DCPS=%CD%\..\..\examples\dcps
FOR /F "delims=" %%i IN (%LIST%) DO (
   echo ============================================
   echo === Running %%i
   echo ============================================
   cd %DCPS%\%%i\%LANG%\Standalone\Windows\Bat
   echo === %DCPS\%%i\%LANG%\Standalone\Windows\Bat
   call myRUN.bat
)
popd