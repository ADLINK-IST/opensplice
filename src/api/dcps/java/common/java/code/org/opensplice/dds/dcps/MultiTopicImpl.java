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


package org.opensplice.dds.dcps;

/** 
 * Implementation of the {@link DDS.MultiTopic} interface. 
 */ 
public class MultiTopicImpl extends MultiTopicBase implements DDS.MultiTopic {

    private static final long serialVersionUID = 5129512816483692741L;
    private String name = null;
    private String type_name = null;
    private DomainParticipantImpl participant = null;
    private TypeSupportImpl typeSupport = null;

    @Override
    protected int deinit ()
    {
        return super.deinit();
    }

    @Override
    public String get_type_name ()
    {
        return type_name;
    }

    @Override
    public String get_name ()
    {
        return name;
    }

    /* see DDS.TopicDescriptionOperations for javadoc */
    @Override
    public DDS.DomainParticipant get_participant ()
    {
        return participant;
    }

    /* see DDS.MultiTopicOperations for javadoc */
    @Override
    public String get_subscription_expression ()
    {
        ReportStack.start();
        ReportStack.report(
            DDS.RETCODE_UNSUPPORTED.value, "MultiTopic not yet supported.");
        ReportStack.flush(this, true);
        return null;
    }

    /* see DDS.MultiTopicOperations for javadoc */
    @Override
    public int get_expression_parameters (DDS.StringSeqHolder expression_parameters)
    {
        int result = DDS.RETCODE_UNSUPPORTED.value;
        ReportStack.start();
        ReportStack.report(
            result, "MultiTopic not yet supported.");
        ReportStack.flush(this, true);
        return result;
    }

    /* see DDS.MultiTopicOperations for javadoc */
    @Override
    public int set_expression_parameters (String[] expression_parameters)
    {
        int result = DDS.RETCODE_UNSUPPORTED.value;
        ReportStack.start();
        ReportStack.report(
            result, "MultiTopic not yet supported.");
        ReportStack.flush(this, true);
        return result;
    }

    public DDS.DataReader create_datareader ()
    {
        return this.typeSupport.create_datareader();
    }

}
