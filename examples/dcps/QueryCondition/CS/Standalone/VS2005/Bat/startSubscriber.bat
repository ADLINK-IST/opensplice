@echo off
rem echo === QueryConditionDataSubscriber

cd ..\Release

if /I "%1" == "" (
  QueryConditionDataSubscriber.exe MSFT
) else (
  QueryConditionDataSubscriber.exe MSFT > ..\Bat\%1
)

cd ..\Bat