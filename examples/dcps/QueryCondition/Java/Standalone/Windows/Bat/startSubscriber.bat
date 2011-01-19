@echo off
rem echo === QueryConditionDataSubscriber

cd ..\exec

if /I "%1" == "" (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";QueryConditionDataSubscriber.jar QueryConditionDataSubscriber MSFT
) else (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";QueryConditionDataSubscriber.jar QueryConditionDataSubscriber MSFT > ..\Bat\%1
)

cd ..\Bat