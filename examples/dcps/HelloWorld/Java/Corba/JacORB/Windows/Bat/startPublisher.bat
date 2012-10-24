@echo off
rem echo === HelloWorldDataPublisher

cd ..\exec

if  "%1" == "" (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";HelloWorldDataPublisher.jar HelloWorldDataPublisher
) else (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";HelloWorldDataPublisher.jar HelloWorldDataPublisher > ..\Bat\%1
)

cd ..\Bat
