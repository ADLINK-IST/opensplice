@echo off
rem echo === ContentFilteredTopicDataPublisher

cd ..\Release

if  "%1" == "" (
  ContentFilteredTopicDataPublisher.exe
) else (
  ContentFilteredTopicDataPublisher.exe > ..\Bat\%1
)

cd ..\Bat