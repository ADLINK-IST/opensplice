@echo off
rem echo === ListenerDataSubscriber

cd ..\Release

if /I "%1" == "" (ListenerDataSubscriber.exe) else (ListenerDataSubscriber.exe > ..\Bat\%1)

cd ..\Bat
