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
/**
 * DDS Errors for Node.js
 * @module ddserr
 */

/**
 * All functions return >= 0 on success
 * @name DDSErrorStrings
 * @param {number} 0 - Success
 * @param {number} 1 - Non specific error
 * @param {number} 2 - Feature unsupported
 * @param {number} 3 - Bad parameter value
 * @param {number} 4 - Precondition for operation not met
 * @param {number} 5 - When an operation fails because of a lack of resources
 * @param {number} 6 - When a configurable feature is not enabled
 * @param {number} 7 - When an attempt is made to modify an immutable policy
 * @param {number} 8 - When a policy is used with inconsistent values
 * @param {number} 9 - When an attempt is made to delete something
 * more than once
 * @param {number} 10 - When a timeout has occured
 * @param {number} 11 - When expected data is not provided
 * @param {number} 12 - When a funciton is called when it should not be
 */
const errorStrings = {
  0: 'Ok',
  1: 'Error',
  2: 'Unsupported',
  3: 'Bad parameter',
  4: 'Preconditions not met',
  5: 'Out of resources',
  6: 'Not enabled',
  7: 'Immutable policy',
  8: 'Inconsistent policy',
  9: 'Already deleted',
  10: 'Timeout',
  11: 'No data',
  12: 'Illegal operation',
};

/**
 * Extract the error number
 * @param {number} e - error value < 0
 * @returns {number} - the extracted error number
 */
function ddsErrNo(e){
  /** DDS_ERR_NO_MASK = 0x000000ff */
  return -e & 0x000000ff;
}

/**
 * Get the error string from the error number
 * @param {number} errno - error number
 * @returns {string} - error string
 */
function ddsErrorMsg(errno) {
  if (errno < 0){
    /** get the extracted error number */
    return errorStrings[ddsErrNo(errno)];
  }
  return errorStrings[errno];
};
exports.ddsErrorMsg = ddsErrorMsg;

/**
 * Class representing dds errors
 */
class DDSError extends Error{

  /**
   * Create an error object
   * @param {number} code
   * @param {string} message
   */
  constructor(code, message){
    var ddsMsg = ddsErrorMsg(code);
    super(message + ddsMsg);
    if (code < 0){
      this.code = ddsErrNo(code);
    } else {
      this.code = code;
    }
  }

  /**
   * Get the error code
   * @returns {number} - the error code
   */
  get ddsErrCode(){
    return this.code;
  }
}

module.exports.DDSError = DDSError;
