@echo off
rem echo === DurabilityDataSubscriber

cd ..\exec

if /I "%2" == "" (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";DurabilityDataSubscriber.jar DurabilityDataSubscriber %1
) else (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";DurabilityDataSubscriber.jar DurabilityDataSubscriber %1 >> ..\Bat\%2
)

cd ..\Bat