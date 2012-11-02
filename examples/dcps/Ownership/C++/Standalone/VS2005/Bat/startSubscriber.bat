@echo off
rem echo === OwnershipDataSubscriber

cd ..\Release

if /I "%1" == "" (
OwnershipDataSubscriber.exe
) else (
OwnershipDataSubscriber.exe > ..\Bat\%1
)

cd ..\Bat
