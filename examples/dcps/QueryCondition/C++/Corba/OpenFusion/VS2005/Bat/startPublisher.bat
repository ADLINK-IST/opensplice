@echo off
rem echo === QueryConditionDataPublisher

cd ..\Release

if /I "%1" == "" ( 
  QueryConditionDataPublisher.exe
) else ( 
  QueryConditionDataPublisher.exe > ..\Bat\%1
)

cd ..\Bat