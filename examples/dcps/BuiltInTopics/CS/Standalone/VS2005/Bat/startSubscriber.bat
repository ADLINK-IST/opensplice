@echo off
rem echo === BuildInTopicsDataSubscriber

cd ..\Release

if /I "%1" == "" (
BuiltInTopicsDataSubscriber.exe
) else (
BuiltInTopicsDataSubscriber.exe > ..\Bat\%1
)

cd ..\Bat
