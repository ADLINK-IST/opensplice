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
'use strict';

const dds = require('vortexdds');
const path = require('path');

module.exports.create = async function(participant, topicName) {

  const idlName = 'ishape.idl';
  const idlPath = __dirname + path.sep + idlName;

  let topicqosprovider = new dds.QoSProvider(
    './DDS_PersistentQoS_All.xml',
    'DDS PersistentQosProfile'
  );

  let typeSupports = await dds.getTopicTypeSupportsForIDL(idlPath);

  let typeSupport = typeSupports.get('ShapeType');

  return participant.createTopicFor(
    topicName,
    typeSupport,
    topicqosprovider.getTopicQos()
  );
};


