@echo off
rem echo === ListenerDataPublisher

cd ..\Release

if /I "%1" == "" (
ListenerDataPublisher.exe
) else (
ListenerDataPublisher.exe > ..\Bat\%1
)

cd ..\Bat
