@echo off
rem echo === OwnershipDataSubscriber

cd ..\exec

if /I "%1" == "" (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";OwnershipDataSubscriber.jar OwnershipDataSubscriber
) else (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";OwnershipDataSubscriber.jar OwnershipDataSubscriber > ..\Bat\%1
)

cd ..\Bat
