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
const ddserr = require('./ddserr');
const expect = require('chai').expect;

describe('DDS Error Class tests', function() {
  it('error code matches error message', function() {
    expect(ddserr.ddsErrorMsg(0)).to.be.equal('Ok');
    expect(ddserr.ddsErrorMsg(1)).to.be.equal('Error');
    expect(ddserr.ddsErrorMsg(2)).to.be.equal('Unsupported');
    expect(ddserr.ddsErrorMsg(3)).to.be.equal('Bad parameter');
    expect(ddserr.ddsErrorMsg(4)).to.be.equal('Preconditions not met');
    expect(ddserr.ddsErrorMsg(5)).to.be.equal('Out of resources');
    expect(ddserr.ddsErrorMsg(6)).to.be.equal('Not enabled');
    expect(ddserr.ddsErrorMsg(7)).to.be.equal('Immutable policy');
    expect(ddserr.ddsErrorMsg(8)).to.be.equal('Inconsistent policy');
    expect(ddserr.ddsErrorMsg(9)).to.be.equal('Already deleted');
    expect(ddserr.ddsErrorMsg(10)).to.be.equal('Timeout');
    expect(ddserr.ddsErrorMsg(11)).to.be.equal('No data');
    expect(ddserr.ddsErrorMsg(12)).to.be.equal('Illegal operation');
  });

  it('invalid error code', function(){
    expect(ddserr.ddsErrorMsg(45)).to.be.equal(undefined);
    expect(ddserr.ddsErrorMsg('hello')).to.be.equal(undefined);
  });

  it('negative error code (-513)', function(){
    let err = new ddserr.DDSError(-513);
    expect(err.ddsErrCode).to.be.equal(1);
    expect(ddserr.ddsErrorMsg(-513)).to.be.equal('Error');
  });

  it('ddsErrCode test', function(){
    let err = new ddserr.DDSError(2);
    expect(err.ddsErrCode).to.be.equal(2);
  });
});
