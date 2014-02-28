@echo off

set cygwin_root=C:\Cygwin
set osplenvs=%TEMP%\tmpospl.txt


REM Setup Prism Environment

del /F %osplenvs% >NUL 2>&1
if exist C:\cygwin\home\dds\INSTALLED_FOR_DDS\STAF3411\data\dds\OpenSpliceDDS (
  %cygwin_root%\bin\find.exe /cygdrive/C/cygwin/home/dds/INSTALLED_FOR_DDS/STAF3411/data/dds/OpenSpliceDDS -name release.bat -exec %cygwin_root%\bin\cygpath -w '{}' ';' >%osplenvs%
)
rem if exist D:\PrismTech (
rem   %cygwin_root%\bin\find.exe /cygdrive/D/PrismTech -name release.bat -exec %cygwin_root%\bin\cygpath -w '{}' ';'rem  >%osplenvs%
rem )
rem if exist "C:\Program Files\PrismTech" (
rem  %cygwin_root%\bin\find.exe "/cygdrive/C/Program Files/PrismTech" -name release.bat -exec %cygwin_root%\bin\cygpath -w '{}' ';' >>%osplenvs%
rem )
if not exist %osplenvs% (
  echo "No installed OSPL found. Starting without it."
  goto setup_staf
)

set count=1
set choices=
for /F "delims=*" %%l in (%osplenvs%) do call :count_subroutine "%%l"
set choices=%choices%0
echo 0 : No OSPL environment (if building from source).
goto do_choice

:count_subroutine
echo %count% : %1
set choices=%choices%%count%
set /A count+=1
goto :eof

:do_choice
choice /M "Choose an environment" /C %choices%
set sel=%ERRORLEVEL%

set count=1
for /F "delims=*" %%l in (%osplenvs%) do call :source_prism_subroutine "%%l"
del /F %osplenvs% >NUL 2>&1
goto setup_staf

:source_prism_subroutine
if %count%==%sel% (
  call %1
  goto :eof
)
set /A count+=1
goto :eof


REM Setup STAF Environment

:setup_staf
if defined STAFCONVDIR goto source_staf
echo "STAF is not setup"
goto need_vs

:source_staf
echo Setting STAF environment for "%USERNAME%"
call "%STAFCONVDIR%\..\STAFEnv.bat" %USERNAME%
goto need_vs


REM Setup MS Visual Studio Environment

:need_vs
choice /M "Do you need VS vars (if building from source choose N)?" /C YN
set sel=%ERRORLEVEL%


if %sel%==2 goto start_cygwin

if defined VS80COMNTOOLS goto source_vs80 
if defined VS90COMNTOOLS goto source_vs90 
if defined VS100COMNTOOLS goto source_vs100
echo "Visual Studio is not setup."
goto start_cygwin

:source_vs80
call "%VS80COMNTOOLS%\vsvars32.bat"
goto start_cygwin

:source_vs90
call "%VS90COMNTOOLS%\vsvars32.bat"
goto start_cygwin

:source_vs100
call "%VS100COMNTOOLS%\vsvars32.bat"
goto start_cygwin

REM Start Cygwin

:start_cygwin
chdir /D %cygwin_root%\bin

REM Clean temporaries
set cygwin_root=
set osplenvs=
set count=
set choices=
set sel=

bash --login -i
goto end

:end
REM end
