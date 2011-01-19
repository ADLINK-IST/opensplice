@echo OFF
pushd %~dp0
rem usage : build_all_standalone.bat examples_list language
rem language = C++ | C  | Java
if "%2" == "" (
  echo "*** usage : run_all_standalone.sh examples_list language"
)
SET LIST=%1
SET LANG=%2
SET DCPS=%CD%\..\..\examples\dcps
ospl stop
SET SLEEP2=ping 127.0.0.1 -n 2
%SLEEP2% >nul
FOR /F "delims=" %%i IN (%LIST%) DO (
   echo ============================================
   echo === Building %%i
   echo ============================================
  if "%LANG%" == "Java" (
     cd %DCPS%\%%i\%LANG%\Standalone\Windows
     echo === Entering %DCPS%\%%i\%LANG%\Standalone\Windows
    ) else (
     cd %DCPS%\%%i\%LANG%\Standalone\Windows\VS2005
     echo === Entering %DCPS%\%%i\%LANG%\Standalone\Windows\VS2005
   )
   del /F /Q  build.log
   del /S /Q  bld 
   del /S /Q  exec
   call BUILD.bat >> build.log

   FIND /C "Err" build.log > nul
   if "%ERRORLEVEL%" == "0" (echo "=== Build OK") else (echo "*** Build NOK")
)
popd