@echo off
cd ..\exec

if /I "%1" == "" ( 
   java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";LifecycleDataSubscriber.jar LifecycleDataSubscriber
) else (
   java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";LifecycleDataSubscriber.jar LifecycleDataSubscriber > ..\Bat\%1 
)
cd ..\Bat
