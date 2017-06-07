/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
 * According to the DDS specification Topics have a multiple inheritance relationshipi
 * with Entity and TopicDescription. Multiple inheritance however is not supported by Java.
 * The TopicDescription class is therefore implemented as an interface in both the DDS and
 * the OpenSplice package.
 *
 * The TopicDescription in the DDS package declares the operations specified in the specification,
 * this interface is generated from IDL.
 *
 * The TopicDescription in the OpenSplice package declares the OpenSplice private operations
 * that aren't part of the IDL and subsequently not part of the DDS package TopicDescription.
 *
 * The Topic, ContentFilteredTopic and MultiTopic will implement both TopicDescription interfaces.
 */

package org.opensplice.dds.dcps;

public interface TopicDescription
{
  DDS.DataReader create_datareader();
  DDS.DataReaderView create_dataview();
  int keep();
  int free();
}

