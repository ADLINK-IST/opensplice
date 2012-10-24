@echo off

rem temp for dev
call "C:\Program Files (x86)\PrismTech\OpenSpliceDDS\V5.3.0\HDE\x86.win32\release.bat"

SET DOTNET_FRAMEWORK_20="%SystemRoot%\Microsoft.NET\Framework\v2.0.50727\"

echo =====================================================
echo ===      Building DCPS VC++2005 examples          ===
echo =====================================================
echo ----------------------------------------------------
echo --- Building ContentFilteredTopic example
echo ----------------------------------------------------

echo "done."

"%DOTNET_FRAMEWORK_20%\MSBuild.exe" ContentFilteredTopic.sln /t:Rebuild /p:Configuration=Release
"%DOTNET_FRAMEWORK_20%\MSBuild.exe" ContentFilteredTopic.sln /t:Rebuild /p:Configuration=Debug

echo "done."
pause
