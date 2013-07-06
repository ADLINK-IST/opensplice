#!/bin/sh

# Set subscriber executable and ID:
export SUB_EXEC=dds2466_sub
export SUB_ID=$1

# Path to the subscriber executable:
export PATH=`dirname $SUB_EXEC`:$PATH

$SUB_EXEC | tee subscriber$1.log 2>&1
