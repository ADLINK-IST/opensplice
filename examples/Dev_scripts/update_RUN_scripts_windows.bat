@echo OFF
rem usage update_RUN_scripts_standalone.bat examples_list -from language -to language
rem language = C  | Java | CS

if "%5" == "" GOTO Err
if NOT "%2" == "-from" GOTO Err
if NOT "%4" == "-to" GOTO Err

SET LIST=%1
SET FROM_LANG=%3
SET TO_LANG=%5

if "%FROM_LANG%" == "Java" (
SET FROM_DIR=Standalone\Windows\Bat
) else (
SET FROM_DIR=Standalone\VS2005\Bat
)
if "%TO_LANG%" == "Java" (
SET TO_CORBA_DIR=Corba\JacORB\Windows\Bat
SET TO_SA_DIR=Standalone\Windows\Bat
) else (
SET TO_CORBA_DIR=Corba\OpenFusion\VS2005\Bat
SET TO_SA_DIR=Standalone\VS2005\Bat
)

SET DCPS=%CD%\..\..\examples\dcps

FOR /F "delims= " %%i IN (%LIST%) DO (
   echo ============================================
   echo === %%i
   echo ============================================
   xcopy /Y /F  %DCPS%\%%i\%FROM_LANG%\%FROM_DIR%\RUN.bat %DCPS%\%%i\%TO_LANG%\%TO_SA_DIR%\RUN.bat
   xcopy /Y /F %DCPS%\%%i\%FROM_LANG%\%FROM_DIR%\RUN.bat %DCPS%\%%i\%TO_LANG%\%TO_CORBA_DIR%\RUN.bat
)
xcopy /Y /F  %DCPS%\Durability\%FROM_LANG%\%FROM_DIR%\start.bat %DCPS%\Durability\%TO_LANG%\%TO_SA_DIR%\start.bat
xcopy /Y /F %DCPS%\Durability\%FROM_LANG%\%FROM_DIR%\start.bat %DCPS%\Durability\%TO_LANG%\%TO_CORBA_DIR%\start.bat
GOTO end
:usage
  echo "*** usage : update_RUN_scripts_standalone.bat examples_list -from language -to language"
:end


