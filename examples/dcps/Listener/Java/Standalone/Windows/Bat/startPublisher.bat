@echo off
rem echo === ListenerDataPublisher

cd ..\exec

if /I "%1" == "" (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";ListenerDataPublisher.jar ListenerDataPublisher
) else (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";ListenerDataPublisher.jar ListenerDataPublisher > ..\Bat\%1
)

cd ..\Bat
