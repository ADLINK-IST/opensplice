@echo off
rem echo === BuildInTopicsDataSubscriber

cd ..\exec

if /I "%1" == "" (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";BuildInTopicsDataSubscriber.jar BuildInTopicsDataSubscriber
) else (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";BuildInTopicsDataSubscriber.jar BuildInTopicsDataSubscriber > ..\Bat\%1
)

cd ..\Bat
