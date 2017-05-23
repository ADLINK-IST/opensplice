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

package DDS;


public final class PublisherQos 
{
  public DDS.PresentationQosPolicy presentation = new DDS.PresentationQosPolicy();
  public DDS.PartitionQosPolicy partition = new DDS.PartitionQosPolicy();
  public DDS.GroupDataQosPolicy group_data = new DDS.GroupDataQosPolicy();
  public DDS.EntityFactoryQosPolicy entity_factory = new DDS.EntityFactoryQosPolicy();

  public PublisherQos ()
  {
  } // ctor

  public PublisherQos (DDS.PresentationQosPolicy _presentation,
                       DDS.PartitionQosPolicy _partition,
                       DDS.GroupDataQosPolicy _group_data,
                       DDS.EntityFactoryQosPolicy _entity_factory)
  {
    presentation = _presentation;
    partition = _partition;
    group_data = _group_data;
    entity_factory = _entity_factory;
  } // ctor

} // class PublisherQos
