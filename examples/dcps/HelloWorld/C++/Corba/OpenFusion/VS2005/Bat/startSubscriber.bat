@echo OFF
cd ..\Release
echo.
if /I NOT "%1" == "" (HelloWorldDataSubscriber.exe > ..\Bat\%1) else (HelloWorldDataSubscriber.exe)
cd ..\Bat
