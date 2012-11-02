@echo off
rem echo === DurabilityDataPublisher

cd ..\Release
echo === DurabilityDataPublisher %*
DurabilityDataPublisher.exe %*
cd ..\Bat