# <publisher_name> <ownership_strength> <nb_iterations> <stop_subscriber_flag>
echo === starting publisher "pub1" with ownership strength 5
../exec/OwnershipDataPublisher pub1 5 40 1&
echo === Waiting 2 seconds ...
sleep 2
echo === starting publisher "pub2" with ownership strength 10
# <publisher_name> <ownership_strength> <nb_iterations> <stop_subscriber_flag>
../exec/OwnershipDataPublisher pub2 10 5 0
