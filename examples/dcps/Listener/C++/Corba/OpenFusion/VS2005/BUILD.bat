@echo off
call "C:\Program Files\PrismTech\OpenSpliceDDS\V5.3.0p3\HDE\x86.win32/release.bat"
rem TAO
set TAO_ROOT=C:\Devel\TAO161_0-Windows-vc8-inline-32bit-OpenSpliceDDS_v5.1_UNSUPPORTED_100324
set PATH=%TAO_ROOT%\bin;%PATH%
rem JacORB
set JACORB_HOME="C:\Program Files\PrismTech\OpenFusionV4"
set PATH=%JACORB_HOME%\bin;%PATH%

SET DOTNET_FRAMEWORK_20="%SystemRoot%\Microsoft.NET\Framework\v2.0.50727\"

echo =====================================================
echo ===      Building DCPS VC++2005 examples          ===
echo =====================================================
echo ----------------------------------------------------
echo --- Building Listener C++ Corba/OpenFusion example
echo ----------------------------------------------------

echo --- Release ----------------------------------------
"%DOTNET_FRAMEWORK_20%\MSBuild.exe" Listener.sln /t:Rebuild /p:Configuration=Release

echo "done."

rem echo --- Debug ------------------------------------------
rem "%DOTNET_FRAMEWORK_20%\MSBuild.exe" Listener.sln /t:Rebuild /p:Configuration=Debug

rem echo "done."
echo =====================================================

pause
