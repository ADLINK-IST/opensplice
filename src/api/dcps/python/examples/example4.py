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
'''
Demonstrate finding DDS topics 'over-the-wire'. That is, find topics registered
in other processes, and read and write samples to those topics

@author: ADLINK
'''

import subprocess
import time
import ddsutil
import dds

def demo_over_the_wire_topics():
    print('Starting OpenSplice Tester, which registers a topic we don''t have locally...')
    # This runs OpenSplice Tester headlessly, and runs a Tester 'scenario' to periodically
    # write data to the OsplTestTopic topic.
    subprocess.call(['ospltest', '-headless', '-e', '-s', 'PythonExample4.sd'])
    print('   Waiting 15s for it to start...')
    time.sleep(15)
    print('   Finished waiting, continuing...')

    print('Connecting to DDS domain...')
    dp = dds.DomainParticipant()

    print('Finding OsplTestTopic...')
    found_topic = dp.find_topic('OsplTestTopic')

    print('Registering OsplTestTopic locally')
    local_topic = ddsutil.register_found_topic_as_local(found_topic)

    print('Getting Python classes for the found topic...')
    gen_info = ddsutil.get_dds_classes_for_found_topic(found_topic)
    OsplTestTopic = gen_info.get_class(found_topic.type_name)
    Tstate = gen_info.get_class('ospllog::Tstate')

    print('Creating sample data to write...')
    data = OsplTestTopic(id=11,index=22, x=1.2, y=2.3, z= 3.4, t=9.8, state=Tstate.init, description='Hello from Python')

    print('Creating readers and writers...')
    pub = dp.create_publisher()
    wr = pub.create_datawriter(local_topic, found_topic.qos)
    sub = dp.create_subscriber()
    rd = sub.create_datareader(local_topic, found_topic.qos)

    print('Writing sample data...')
    wr.write(data)
    print('Wrote: %s' % (str(data)))

    print('Waiting a bit so Tester has time to see our data...')
    time.sleep(3)


    print('Reading data...')
    l = rd.take(10)
    for (sd, si) in l:
        if si.valid_data:
            print('Read: %s' % (str(sd)))

    print('All Done!!!')

if __name__ == '__main__':
    demo_over_the_wire_topics()