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
/* eslint-env node, mocha */

const libs = require('./libs');
const expect = require('chai').expect;
const os = require('os');

describe('DDS Shared Libraries tests', function() {
  it('os platform libraries', function() {
    const platform = os.platform();
    const libdds = libs.libdds;

    platform === 'linux' ? expect(libdds).to.be.equal('libdcpsc99') :
      platform === 'win32' ? expect(libdds).to.be.equal('dcpsc99') :
        expect(libdds).to.be.undefined;
  });
});
