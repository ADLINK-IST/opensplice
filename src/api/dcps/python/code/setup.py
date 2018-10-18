#
#                         Vortex OpenSplice
#
#   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
#   Technology Limited, its affiliated companies and licensors. All rights
#   reserved.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
import os
import sys
import warnings
from re import search
from subprocess import check_call
from setuptools import setup
from setuptools.extension import Extension
from Cython.Build import cythonize

if not os.getenv('OSPL_HOME'):
    raise Exception('Environment variable OSPL_HOME not defined.')

if os.getenv('SPLICE_TARGET'):
    version = os.environ['PACKAGE_VERSION']
    idl_path = os.environ['OSPL_HOME_NORMALIZED']+'/etc/idl/dds_builtinTopics.idl'
    LDFLAGS=[os.environ['OSPL_HOME_NORMALIZED']+'/lib/'+os.environ['SPLICE_TARGET']]
    CINCS=[os.environ['OSPL_HOME_NORMALIZED']+'/src/api/dcps/c99/include',
            os.environ['OSPL_HOME_NORMALIZED']+'/src/api/dcps/sac/include',
            os.environ['OSPL_HOME_NORMALIZED']+'/src/api/dcps/c99/bld/'+os.environ['SPLICE_TARGET'],
            os.environ['OSPL_HOME_NORMALIZED']+'/src/abstraction/os/include',
            # following includes needed for builtintopics files
            os.environ['OSPL_HOME_NORMALIZED']+'/src/user/include',
            os.environ['OSPL_HOME_NORMALIZED']+'/src/kernel/include',
            os.environ['OSPL_HOME_NORMALIZED']+'/src/osplcore/bld/'+os.environ['SPLICE_TARGET'],
            os.environ['OSPL_HOME_NORMALIZED']+'/src/database/database/include']
else:
    with open(os.environ['OSPL_HOME']+'/etc/RELEASEINFO') as f:
        version = search('PACKAGE_VERSION=(.*)\n', f.read()).group(1)
    idl_path = os.environ['OSPL_HOME']+'/etc/idl/dds_builtinTopics.idl'
    LDFLAGS=[os.environ['OSPL_HOME']+'/lib']
    CINCS=[os.environ['OSPL_HOME']+'/include/dcps/C/C99',
            os.environ['OSPL_HOME']+'/include/dcps/C/SAC',
            os.environ['OSPL_HOME']+'/include/sys']

print('Executing idlpp')
check_call(["idlpp", "-l", "c99", idl_path])
print('idlpp success')

extensions = [Extension('dds',
                        ['dds.pyx',
                            'dds_builtinTopicsDcps.c',
                            'dds_builtinTopicsSacDcps.c',
                            'dds_builtinTopicsSplDcps.c'],
                        libraries=['dcpsc99','dcpssac','ddskernel'],
                        library_dirs=LDFLAGS,
                        include_dirs=CINCS,
                        extra_compile_args=['-DOSPL_BUILD_DCPSSAC'])]

extensions = cythonize(extensions)

with warnings.catch_warnings():
    # On Windows, running bdist_wheel throws RuntimeWarnings about Config variables not being set,
    # claiming Python ABI tag may be incorrect.
    if 'bdist_wheel' in sys.argv:
        warnings.filterwarnings('ignore', '(.*Py_DEBUG|.*WITH_PYMALLOC)', RuntimeWarning)
    setup(
        name = 'dds',
        version = version,
        description = 'A python implementation of the OMG DDS Data-Centric Publish-Subscribe (DCPS) API, '
                      'for use with Vortex OpenSplice DDS middleware.',
        url = 'http://vortex.adlinktech.com',
        author = 'ADLINK Technology',
        author_email = 'ist_support@adlinktech.com',
        py_modules = ['ddsutil'],
        ext_modules = extensions
    )

