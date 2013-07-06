#!/bin/sh

# Set publisher executable and ID:
export PUB_EXEC=dds2466_pub
export PUB_ID=$1
export ANOTHER_PUB_ID=$2

# Path to the publisher executable:
export PATH=`dirname $PUB_EXEC`:$PATH

# Run subscriber:
$PUB_EXEC -ORBListenEndpoints iiop://$HOSTNAME:$PUB_PORT -ORBInitRef $PUB_EXEC$ANOTHER_PUB_ID=corbaloc:iiop:$ANOTHER_PUB_HOST:$ANOTHER_PUB_PORT/$PUB_EXEC$ANOTHER_PUB_ID | tee publisher$1.log 2>&1
