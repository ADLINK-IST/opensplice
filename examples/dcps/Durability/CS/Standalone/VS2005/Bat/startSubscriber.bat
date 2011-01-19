@echo off
rem echo === DurabilityDataSubscriber

cd ..\Release

if /I "%2" == "" (
DurabilityDataSubscriber.exe %1
) else (
DurabilityDataSubscriber.exe %1 >> ..\Bat\%2
)

cd ..\Bat