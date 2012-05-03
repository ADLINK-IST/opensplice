@echo off
rem echo === LifecycleDataSubscriber
cd ..\Release
if /I "%1" == "" (LifecycleDataSubscriber.exe) else (LifecycleDataSubscriber.exe > ..\Bat\%1)
cd ..\Bat
