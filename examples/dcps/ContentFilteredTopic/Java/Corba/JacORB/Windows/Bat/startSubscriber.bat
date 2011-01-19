@echo off
rem echo === ContentFilteredTopicDataSubscriber

cd ..\exec

if /I "%1" == "" (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";ContentFilteredTopicDataSubscriber.jar ContentFilteredTopicDataSubscriber GE
) else (
  java -classpath "%OSPL_HOME%\jar\dcpssaj.jar";ContentFilteredTopicDataSubscriber.jar ContentFilteredTopicDataSubscriber GE > ..\Bat\%1
)

cd ..\Bat