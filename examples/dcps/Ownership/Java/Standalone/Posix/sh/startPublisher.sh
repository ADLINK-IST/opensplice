# <publisher_name> <ownership_strength> <nb_iterations> <stop_subscriber_flag>
echo ../exec/OwnershipDataPublisher $*
java -classpath $OSPL_HOME/jar/dcpssaj.jar:../exec/OwnershipDataPublisher.jar OwnershipDataPublisher $*
