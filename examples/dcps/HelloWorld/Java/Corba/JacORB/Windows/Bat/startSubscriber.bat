@echo off
rem echo === HelloWorldDataSubscriber

cd ..\exec

if /I "%1" == "" (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";HelloWorldDataSubscriber.jar HelloWorldDataSubscriber
) else (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";HelloWorldDataSubscriber.jar HelloWorldDataSubscriber > ..\Bat\%1
)

cd ..\Bat
