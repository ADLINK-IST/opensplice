[![Build Status](https://dev.azure.com/ADLINK-GDE/Vortex%20OpenSplice%20Community%20Edition/_apis/build/status/ADLINK-IST.opensplice?branchName=master)](https://dev.azure.com/ADLINK-GDE/Vortex%20OpenSplice%20Community%20Edition/_build/latest?definitionId=7&branchName=master)

# Vortex OpenSplice Community Edition

Vortex OpenSplice a full implementaiton of the **OMG DDS Standard** licensed under **Apache 2**. The DDS standard is used today across a large range of application domains ranging from autonomous vehicles, medical devices, robotic platforms, software defined networking, network switches, IoT Gateways, military and aerospace systems, Air Traffic Control and Management, smart grids, smart farms, etc.

## Building

### POSIX / Linux

#### POSIX / Linux Minimal Build Required Tools

**NOTE**: The list of tools below is the minimal set required to build Vortex OpenSplice Community Edition without CORBA ORB collocation support. See the required tools section below for details of the additional dependencies to build Vortex OpenSplice Community Edition with support for sharing DDS types with a C++ or Java CORBA ORB.

The following tools should be installed and available on the machine executable / library search path:

- bash shell
- A suitable C / C++ compiler, e.g:
    - gcc - 3.2.x or above
    - Sun Studio - At this time only v12 of this compiler has been tested
- gmake - 3.80 or above.
- gawk - Any version is acceptable.
- flex - Any version is acceptable.
- bison - Version 2.7 or above is required.
- perl - Version 5.8 or above is required.
- gSOAP - This is optional. Version 2.7 or above is acceptable. If omitted the cmsoap services is not supported.
- Java SDK - This is optional. SDK (>1.6 < 1.9) are working java version > 1.8 does not work. If omitted no Java APIs or tools will be generated.
- Maven - This is optional. Version 3.0 or above is acceptable. If omitted no Java APIs or tools will be generated.
- Doxygen - This is optional. If omitted the documentation for the C# and I.S.O. C++ APIs will not be generated. Version 1.8 or greater is recommended for ideal results. (v6 of Vortex OpenSplice Community Edition onwards only)
- Protobuf - This is optional. Version 2.6.x is required. If omitted data types modeled in Google Protocol Buffers will not be supported
- Qt4 - Highly optional. Its only use is to build the iShapes demonstration application into the distribution, which can be used to illustrate DCPS use and verify interoperability. (v6 of Vortex OpenSplice Community Edition onwards only). See demos/iShapes/README.md.

#### POSIX / Linux Build with Optional CORBA Collocation Support Required Tools

In addition to the above dependencies the following may also be optionally installed and used to enable support for sharing data types with a CORBA ORB.

- **TAO** - See the installation instructions for how to install and configure the build environment.
    - Install and set TAO_ROOT to the directory that the TAO C++ ORB is installed in, before sourcing configure to enable C++ CORBA support.
- **JacORB** - See the installation instructions for how to install and configure the build environment.
    - Install and set JACORB_HOME to the directory that the JacORB Java ORB is installed in, before sourcing configure to enable Java CORBA support.

#### POSIX / Linux Steps to Build Vortex OpenSplice Community Edition from Source

- Set environment variables so that the above tools can be located:
    - Optionally set GSOAPHOME to the location of the gSOAP toolkit
    - Optionally set either or both TAO_ROOT or JACORB_HOME if you wish to build CORBA support. Otherwise: leave either or both unset. See above.
    - Optionally set JAVA_HOME - set to the directory that a SDK (>1.6 < 1.9) is installed in.
	- Optionally set M2_HOME - set to the directory that Apache Maven is installed in.
    - Optionally set PROTOBUF_HOME - set to the directory where Google Protocol Buffer is installed.
    - All other tools will be located from the machine $PATH
- Source the Vortex OpenSplice configure script to set-up your build environment. N.B. You must source this script each time you wish to configure a new shell as a build environment. The configuration choices are not persistent.
- Choose the target platform configuration.
- Invoke `make` to compile Vortex OpenSplice Commmunity Edition
- Invoke `make install` to build a Vortex OpenSplice Community Edition Distribution

```
$ export TAO_ROOT=`<TAO location>`
$ export JACORB_HOME=`<JacORB location>`
$ export JAVA_HOME=`<Oracle Java SDK location>`
$ export GSOAPHOME=`<gSOAP location>`
$ source ./configure
$ make
$ make install
```

#### Example

```
sm@plutus:~/opensplice> export GSOAPHOME=/usr/gsoap/2.8.60
sm@plutus:~/opensplice> source configure

Setup at 10:37:10 for Vortex - Version 6.9.181127OSS - Date 2018-12-06

GCC: OK - Using version 7
  NOTE - Enable link-time optimizations (for release build)
GLIBC: version 2.27
MAKE: OK - using GNU Make 4.1
Perl: OK - using perl version='5.26.1';
Qt: On. Using QT tools from the path (with -qt4 suffix).
GAWK: OK - using GNU Awk 4.1.4, API: 1.1 (GNU MPFR 4.0.1, GNU MP 6.1.2)
BISON: OK - using 3.0.4
FLEX: OK - using 2.6.4
JAVAC: OK - using JAVAC version 1.8.0_191
  JAVA_HOME is /usr/lib/jvm/java-8-openjdk-amd64
GMCS: Warning - No gmcs compiler found
   gmcs C# compiler not found, disabling SACS api build.
TAO: Warning - No TAO found
   TAO environment not set, disabling TAO related features.
JACORB: Warning - JACORB_HOME not set
   JACORB environment not set, disabling JACORB related features.
GSOAP: OK - using GSOAP version 2.8.60
  setting GSOAPHOME to /usr
DOXYGEN: OK - no doxygen installed, will attempt to retreive docs from git repo
GOOGLE PROTOCOL BUFFERS: PROTOBUF_HOME has not been set
Warning - Protobuf compiler environment not set, building of all protobuf related features is disabled.
C99: OK - supported
Python3: OK - Using /usr/bin/python3 v3.6.7
    Warning: Cython module not found. Cannot build Python DCPS API
    Warning: wheel module not found. Cannot build Python DCPS API
NODEJS: nodejs install dir not found - skipping build of NodeJS DCPS API and unit tests.
MAVEN: OK - Using Apache Maven 3.5.2
Key-value store implementations - SQLITE: Warning - Not found, LEVELDB: Warning - Not found
Configuration OK

Variable Setup
SPLICE_TARGET = x86_64.linux-dev
SPLICE_HOST = x86_64.linux-dev
OSPL_HOME = /home/sm/opensplice
SPLICE_ORB =

sm@plutus:~/opensplice> make && make install
```
### Windows

#### Windows Minimal Build Required Tools

**NOTE**: The below list of tools is the minimal set required to build Vortex OpenSplice Community Edition without CORBA ORB collocation support. See the required tools section below for details of the additional dependencies to build Vortex OpenSplice Community Edition with support for sharing DDS types with a C++ or Java CORBA ORB.

- Microsoft Visual Studio 2008 to Microsoft Visual Studio 2017
- cygwin - The latest available version is recommended. The following packages should be installed in addition to the recommended base selection:
  - gcc-core
  - gmake
  - perl
  - bison
  - flex
  - gawk
  - zip
  - unzip
  - doxygen (Optional - used for C# & ISO C++ API documentation)
- gSOAP - This is optional. Version 2.7 or above is acceptable.
- Java SDK -  This is optional. SDK (>1.6 < 1.9) or above is acceptable. If omitted no Java APIs or tools will be generated.
- Maven - This is optional. Version 3.0 or above is acceptable. If omitted no Java APIs or tools will be generated.
- Protobuf- This is optional. Version 2.6.x is required. If omitted data types modeled in Google Protocol Buffers will not be supported
- Qt4 - Highly optional. Its only use is to build the iShapes demonstration application into the distribution, which can be used to illustrate DCPS use and verify interoperability. (v6 of Vortex OpenSplice Community Edition onwards only). See demos/iShapes/README.md.

#### Windows Build with Optional CORBA Collocation Support Required Tools
In addition to the above dependencies the following may also be optionally installed and used to enable support for sharing data types with a CORBA ORB.

- TAO - See the installation instructions for how to install and configure the build environment.
  - Install and set TAO_ROOT to the directory that the TAO C++ ORB is installed in, before sourcing configure to enable C++ CORBA support.
- JacORB - See the installation instructions for how to install and configure the build environment.
  - Install and set JACORB_HOME to the directory that the JacORB Java ORB is installed in, before sourcing configure to enable Java CORBA support.

#### Windows Steps to build Vortex OpenSplice Community Edition from Source
- In a Cygwin shell set environment variables so that the required build tools can be located.
	- VS_HOME - set to the installation directory of Visual Studio 2008 or 2015.
	- WINDOWSSDKDIR - set to the location of the Windows SDK directory.
  - GSOAPHOME - set to the location of the gSOAP toolkit
	- Optionally set either or both TAO_ROOT or JACORB_HOME if you wish to build CORBA support. Otherwise: leave either or both unset. See above.
	- Optionally set JAVA_HOME - set to the directory that an Oracle SDK (>1.6) is installed in.
	- Optionally set QTDIR and PROTOBUF_HOME
	- All other tools will be located from the cygwin shell $PATH
- Unset TMP and TEMP (lower and upper case) otherwise you may encouter error 256 while building the Vortex OpenSplice Community Edition ISO C++ API. This is a known compatibility issue between Cygwin and Visual Studio. This can be done permanently by editing your /etc/profile and commenting out the setting of the variables.
- Source the Vortex OpenSplice configure script to set-up your build environment. N.B. You must source this script each time you wish to configure a new shell as a build environment. The configuration choices are not persistent.
- Choose the target platform configuration. N.B - 32/64 bit debug (dev) or release (release) builds are available.
	- Note that if you wish to build 'dev' and you wish to build C++ CORBA support then you must have installed a debug version of TAO. If you wish to build release then the TAO installed and indicated by the TAO_ROOT variable must also be release.
- Invoke `make` to build Vortex OpenSplice Community Edition
- Invoke `make install` to build a Vortex OpenSplice Community Edition Distribution

#### Example
```
sm@plutus ~/opensplice
$ export VS_HOME='/cygdrive/C/Program Files (x86)/Microsoft Visual Studio 14.0'

sm@plutus ~/opensplice
$ export WINDOWSSDKDIR='/cygdrive/C/Program Files (x86)/Windows Kits/10'

sm@plutus ~/opensplice
$ export GSOAPHOME=/home/dds/INSTALLED_FOR_DDS/gsoap-v2.7

sm@plutus ~/opensplice
$ export JACORB_HOME=/home/dds/INSTALLED_FOR_DDS/JacORB-v2.3.1.0

sm@plutus ~/opensplice
$ export JAVA_HOME='/cygdrive/C/Program Files/Java/jdk1.8.0_191'

sm@plutus ~/opensplice
$ export QTDIR=/home/dds/INSTALLED_FOR_DDS/Qt/4.8.4_vs2010_64bit

sm@plutus ~/opensplice
$ export PROTOBUF_HOME=/home/dds/INSTALLED_FOR_DDS/protobuf-2.6.0-vs2015

sm@plutus ~/opensplice
$ unset tmp && unset TEMP && unset TMP && unset temp

sm@plutus ~/opensplice
$ source ./configure

Setup at 11:27:13 for Vortex - Version 6.9.181127OSS - Date 2018-12-06

Available targets are:
1 > x86.win32-debug
2 > x86.win32-dev
3 > x86.win32-release
4 > x86_64.win64-debug
5 > x86_64.win64-dev
6 > x86_64.win64-release
Please select a target number:6

VS: OK - using compiler version: 19.00.24215.1 for VS: 2015 and .Net 4.6 and Program Files C:\Program Files (x86)
Visual Studio builder: OK - using devenv.com
C#: OK - using version 1.3.1.60616
MAKE: OK - using GNU Make 4.2.1
Perl: OK - using perl version='5.26.3';
Qt: at_check (1) and QT_VERSION is 4
OK. $QTDIR was /home/dds/INSTALLED_FOR_DDS/Qt/4.8.4_vs2010_64bit,
set to /home/dds/INSTALLED_FOR_DDS/Qt/4.8.4_vs2010_64bit
GAWK: OK - using GNU Awk 4.1.3, API: 1.1 (GNU MPFR 3.1.4, GNU MP 6.1.0)
BISON: OK - using 3.0.4
FLEX: OK - using 2.5.39
JAVAC: OK - using JAVAC version 1.8.0_191
  JAVA_HOME is /cygdrive/c/PROGRA~1/Java/JDK18~1.0_1
TAO: Warning - No TAO found
TAO environment not set, disabling TAO related features.
JACORB: OK
JACORB_HOME is /home/dds/INSTALLED_FOR_DDS/JacORB-v2.3.1.0
GSOAP: OK - using GSOAP version 2.8
  setting GSOAPHOME to /home/dds/INSTALLED_FOR_DDS/gsoap-2.8
DOXYGEN: OK
GOOGLE PROTOCOL BUFFERS: OK - using PROTOC version 2.6.0
PROTOBUF_HOME is /home/dds/INSTALLED_FOR_DDS/protobuf-2.6.0-vs2015
C99: OK - supported
Python3: Warning: Python3 not available. Cannot build Python DCPS API.
MAVEN: OK - Using Apache Maven 3.6.0 (97c98ec64a1fdfee7767ce5ffb20918da4f719f3; 2018-10-24T20:41:47+02:00)
Key-value store implementations - SQLITE: Warning - Not found, LEVELDB: Disabled pending Windows port
Configuration OK

Variable Setup
SPLICE_TARGET = x86_64.win64-release
SPLICE_HOST = x86_64.win64-release
OSPL_HOME = /home/dds/opensplice
SPLICE_ORB =

sm@plutus ~/opensplice
$ make && make install
```

### Rebuilding the Vortex OpenSplice Community Edition Custom Libraries on POSIX / Linux
Binary distributions of Vortex OpenSplice Community Edition are shipped containing pre-built C++ and Java language binding libraries. These are known as the **Vortex OpenSplice Community Edition Custom Libraries**.

For maximum assurance of compatibility with your deployment platform the Vortex OpenSplice Community Edition Custom Libraries of a Vortex OpenSplice Community Edition binary distribution can be rebuilt. In this way you can be confident that these language binding libraries have matching linkage with the toolchain you plan to develop and deploy your application with.

#### Rebuilding the Standalone C++ Custom Libraries on POSIX / Linux
```
cd HDE/<<target_platform>>/custom_lib/
make
```

#### Rebuilding the CORBA Co-habitation C++ Custom Libraries on POSIX / Linux
- Configure the environment for the TAO C++ ORB as per the installation instructions.
- Change directory to the CORBA Co-habitation C++ Custom Libraries source directory in the binary Vortex OpenSplice Community Edition distribution.
- Make the directory.
- Copy the new library into place in he distribution.
```
cd HDE/<<target_platform>>/custom_lib/ccpp/
make
cp -f ./libdcpsccpp.so ../../lib/.
```

#### Rebuilding the Standalone C++ Custom Libraries on Windows
Open the Visual Studio solution file in HDE/<<target_platform>>/custom_lib and rebuild.

### Installing Google Protocol Buffer
Download the source from [https://github.com/google/protobuf/releases](https://github.com/google/protobuf/releases)

```
tar -xzf protobuf-2.6.1.tar.gz
cd protobuf-2.6.1
./configure
make
```
Add protoc to the PATH.

For Windows download a pre-compiled protoc version and put that in your PATH
```
export PROTOBUF_HOME=`pwd`
```

## Documentation

Below are a few links to learning material that will get you started quickly with Vortex OpenSplice and DDS.

- [The DDS Tutorial Booklet](http://bit.ly/2sXqbOG)
- [DDS Tutorial Slides](http://bit.ly/dds-onem2m)
- [DDS Slideshares](http://bit.ly/2sXW6yo)
- [DDS In Action Channel](https://vimeo.com/channels/dds)


## Open Source Add-ons
There are plenty of Open Source add-ons for Vortex OpenSplice and they keep growing almost daily. The main place to look for add one and extensions are [ADLINK-IST's GitHub](https://github.com/ADLINK-IST) repositories. Beside this, below are a few notable extensions:

- **[DDS Tutorial Examples](http://bit.ly/1oAvXhz)**
- **[C++11 Extensions](http://bit.ly/dds-cpp11)**: Support for lambda-based Data Reader listenerns
- **[Moliere](http://bit.ly/moliere-dds)**: [Scala](http://scala-lang.org) APIs for Vortex OpenSplice
- **[pydds](https://github.com/atolab/pydds)**: Python APIs for Vortex OpenSplice

## Related Projects

- The version 2 of the  [Robot Operating System](http://www.ros.org) (ROS2) uses DDS as its underlying communication mechanism.
