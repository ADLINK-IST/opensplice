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

package DDS;


public interface DataWriterListenerOperations  extends DDS.ListenerOperations
{
  void on_offered_deadline_missed (DDS.DataWriter writer, DDS.OfferedDeadlineMissedStatus status);
  void on_offered_incompatible_qos (DDS.DataWriter writer, DDS.OfferedIncompatibleQosStatus status);
  void on_liveliness_lost (DDS.DataWriter writer, DDS.LivelinessLostStatus status);
  void on_publication_matched (DDS.DataWriter writer, DDS.PublicationMatchedStatus status);
} // interface DataWriterListenerOperations
