@echo OFF
pushd %~dp0
rem usage : run_list_windows.bat examples_list language
rem language = C++ | C  | Java
if "%2" == "" (
  echo "*** usage : run_list_windows.bat examples_list language_list"
)
SET LIST=%1
SET LANG_LIST=%2
SET DCPS=%CD%\..\..\examples\dcps
FOR /F "delims=" %%j IN (%LANG_LIST%) DO (
echo == LANG = %%j
FOR /F "delims=" %%i IN (%LIST%) DO (
   echo ============================================
   echo === Running %%i STANDALONE %%j
   echo ============================================
   if "%%j" == "Java" (
   cd %DCPS%\%%i\%%j\Standalone\Windows\Bat
   echo === %DCPS%\%%i\%%j\Standalone\Windows\Bat
   call myRUN.bat
   ) else (
   cd %DCPS%\%%i\%%j\Standalone\VS2005\Bat
   echo === %DCPS%\%%i\%%j\Standalone\VS2005\Bat
   call myRUN.bat
   )
   if "%%j" == "C++" (
   echo ============================================
   echo === Running %%i CORBA OpenFusion %%j
   echo ============================================
   cd %DCPS%\%%i\%%j\Corba\OpenFusion\VS2005\Bat
   echo === %DCPS%\%%i\%%j\Corba\OpenFusion\VS2005\Bat
   call myRUN.bat
   )
   if "%%j" == "Java" (
   echo ============================================
   echo === Running %%i CORBA JacORB %%j
   echo ============================================
   cd %DCPS%\%%i\%%j\Corba\JacORB\Windows\Bat
   echo === %DCPS%\%%i\%%j\Corba\JacORB\Windows\Bat
   call myRUN.bat
   )
   cd %~dp0
)
)
popd