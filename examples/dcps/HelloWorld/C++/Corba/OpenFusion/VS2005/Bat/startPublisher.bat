@echo OFF
cd ..\Release
echo.
if /I NOT "%1" == "" (HelloWorldDataPublisher.exe > ..\Bat\%1) else (HelloWorldDataPublisher.exe)
cd ..\Bat
