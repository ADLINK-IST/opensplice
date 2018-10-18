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
#ifndef ORG_OPENSPLICE_TOPIC_DATA_REPRESENTATION_HPP
#define ORG_OPENSPLICE_TOPIC_DATA_REPRESENTATION_HPP

namespace org { namespace opensplice { namespace topic {

typedef int16_t DataRepresentationId_t;

const DataRepresentationId_t XCDR_REPRESENTATION  = 0;
const DataRepresentationId_t XML_REPRESENTATION   = 0x001;
const DataRepresentationId_t OSPL_REPRESENTATION  = 0x400;
const DataRepresentationId_t GPB_REPRESENTATION   = 0x401;
const DataRepresentationId_t INVALID_REPRESENTATION = 0x7FFF;

}}}

#endif /* ORG_OPENSPLICE_TOPIC_DATA_REPRESENTATION_HPP */
