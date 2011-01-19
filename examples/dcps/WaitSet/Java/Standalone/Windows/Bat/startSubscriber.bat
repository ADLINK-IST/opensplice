@echo off
rem echo === WaitSetDataSubscriber

cd ..\exec

if /I "%1" == "" (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";WaitSetDataSubscriber.jar WaitSetDataSubscriber
) else (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";WaitSetDataSubscriber.jar WaitSetDataSubscriber > ..\Bat\%1
)

cd ..\Bat
