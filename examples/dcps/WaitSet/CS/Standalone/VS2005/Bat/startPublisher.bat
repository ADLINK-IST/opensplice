@echo off
rem echo === WaitSetDataPublisher

cd ..\Release
SET SLEEP2=ping 127.0.0.1 -n 2
%SLEEP2% > nul
if /I "%1" == "" (
  WaitSetDataPublisher.exe
) else (
  WaitSetDataPublisher.exe > ..\Bat\%1
)

cd ..\Bat
