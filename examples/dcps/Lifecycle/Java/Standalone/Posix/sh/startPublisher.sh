# usage LifecyclePublisher<autodispose_unregistered_flag> (true | false)  dispose | unregister | stoppub
echo === LifecycleDataPublisher $1 $2
cd ../exec

java -classpath $OSPL_HOME/jar/dcpssaj.jar:LifecycleDataPublisher.jar LifecycleDataPublisher  $1 $2

cd ../sh
