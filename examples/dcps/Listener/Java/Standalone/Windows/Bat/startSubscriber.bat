@echo off
rem echo === ListenerDataSubscriber

cd ..\exec

if /I "%1" == "" (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";ListenerDataSubscriber.jar ListenerDataSubscriber
) else (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";ListenerDataSubscriber.jar ListenerDataSubscriber > ..\Bat\%1
)

cd ..\Bat
