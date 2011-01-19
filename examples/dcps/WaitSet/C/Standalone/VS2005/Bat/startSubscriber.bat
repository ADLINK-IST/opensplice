@echo off
rem echo === WaitSetDataSubscriber

cd ..\Release

if /I "%1" == "" (
  WaitSetDataSubscriber.exe
) else (
  WaitSetDataSubscriber.exe > ..\Bat\%1
)

cd ..\Bat
