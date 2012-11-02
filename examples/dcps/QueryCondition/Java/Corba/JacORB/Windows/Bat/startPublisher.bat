@echo off
rem echo === QueryConditionDataPublisher

cd ..\exec

if /I "%1" == "" (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";QueryConditionDataPublisher.jar QueryConditionDataPublisher
) else (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";QueryConditionDataPublisher.jar QueryConditionDataPublisher > ..\Bat\%1
)

cd ..\Bat