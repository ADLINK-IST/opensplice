#!/bin/sh

TEST_DIR=`pwd`

cd ../../..
OSPL_ROOT=`pwd`
cd $TEST_DIR

# Set "ospli" location to find Prism MPC configs and ospl_testlib:
export OSPLI_HOME=$OSPL_ROOT

# To build with ospl_testlib (CORBA-based testing framework):
export TAO_ROOT=
export ACE_ROOT=$TAO_ROOT
export LD_LIBRARY_PATH=$TAO_ROOT/lib:$LD_LIBRARY_PATH
export PATH=$TAO_ROOT/bin:$PATH

# Uses "mpc.pl" from $TAO_ROOT/bin:
mwc.pl -type gnuace -include $OSPLI_HOME/MPC/config -noreldefs -template prism

# Make:
make
