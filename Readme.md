# OPENSPLICE

This project is a fork of Opensplice by Primstech to update the arm-9 branch with opensplice 5.5

## Information

## Compilation

### Problems known
N/A

### Beaglebone
- Install Angtorm toolchain
 - . /usr/local/angstrom/environment-setup

- Download Opensplice 5.5 Sources for ARM
 - git clone git://github.com/marcbuils/opensplice.git
 - git checkout -b arm9-linux remotes/origin/arm9-linux

- Install necessary libraries
 - apt-get install bison flex make gawk

- Install Java
 - download java
 - sudo mkdir /usr/local/java
 - sudo tar -xvzf {JAVA_ARCHIVE} /usr/local/java
 - export JAVA_HOME=/usr/local/java/{JAVA_VERSION}

- Install zlib
 - wget http://zlib.net/zlib-1.2.7.tar.gz
 - tar -xvzf zlib-1.2.7.tar.gz
 - cd zlib-1.2.7.tar.gz
 - CC=arm-linux-gcc LDSHARED="arm-linux-gcc -shared -Wl,-soname,libz.so.1" ./configure --shared --prefix=/usr
 - export ZLIB_HOME=$PWD
 - export LDFLAGS_ZLIB=-L$PWD
 - export LDLIBS_ZLIB=-lz
 - export CINCS_ZLIB=-I$PWD

- Compil opensplice
 - cd opensplice
 - . ./configure
 - 2
 - make
 - 1 error => touch src/api/dcps/sacs/bld/arm9.linux2.6-release/dcpssacs.netmodule
 - make

