@echo off
rem echo === ContentFilteredTopicDataSubscriber

cd ..\Release

if /I "%1" == "" (
  ContentFilteredTopicDataSubscriber.exe GE
) else (
  ContentFilteredTopicDataSubscriber.exe GE > ..\Bat\%1
)

cd ..\Bat