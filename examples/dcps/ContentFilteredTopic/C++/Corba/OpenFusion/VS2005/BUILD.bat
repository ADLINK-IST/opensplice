@echo off

SET DOTNET_FRAMEWORK_20="%SystemRoot%\Microsoft.NET\Framework\v2.0.50727\"

echo =====================================================
echo ===      Building DCPS VC++2005 examples          ===
echo =====================================================
echo ----------------------------------------------------
echo --- Building ContentFilteredTopic C++ Corba/OpenFusion example
echo ----------------------------------------------------

echo --- Release ----------------------------------------
"%DOTNET_FRAMEWORK_20%\MSBuild.exe" ContentFilteredTopic.sln /t:Rebuild /p:Configuration=Release

echo "done."

echo --- Debug ------------------------------------------
"%DOTNET_FRAMEWORK_20%\MSBuild.exe" ContentFilteredTopic.sln /t:Rebuild /p:Configuration=Debug

echo "done."
echo =====================================================

pause
