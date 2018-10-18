/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

/*
    IoTTopic helper module.
*/

'use strict';

const path = require('path');
const dds = require('vortexdds');

module.exports.create = async function(participant) {
  const topicName = 'IoTData';
  const idlName = 'dds_IoTData.idl';
  const idlPath = __dirname + path.sep + idlName;

  let typeSupports = await dds.getTopicTypeSupportsForIDL(idlPath);

  let typeSupport = typeSupports.get('DDS::IoT::IoTData');

  return participant.createTopicFor(
    topicName,
    typeSupport,
    null
  );
};
