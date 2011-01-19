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
ospl stop
SET SLEEP2=ping 127.0.0.1 -n 2
%SLEEP2% >nul
FOR /F "delims=" %%i IN (%LIST%) DO (
   echo ============================================
   echo === Running %%i
   echo ============================================
   cd %DCPS%\%%i\%LANG%\Standalone\Windows\Bat
   echo === %DCPS\%%i\%LANG%\Standalone\Windows\Bat
   call RUN.bat
)
popd