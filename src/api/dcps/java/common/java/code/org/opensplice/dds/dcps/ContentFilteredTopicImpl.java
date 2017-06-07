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
 * Implementation of the {@link DDS.ContentFilteredTopic} interface.
 */
public class ContentFilteredTopicImpl
             extends ContentFilteredTopicBase
             implements DDS.ContentFilteredTopic, TopicDescription
{

    private static final long serialVersionUID = -6499519777697875091L;
    private String name = null;
    private String expression = null;
    private String[] parameters = null;
    private TopicImpl relatedTopic = null;
    private DDS.DomainParticipant participant = null;
    private int refCount = 0;
    private boolean deleted = false;

    protected int init (
        DomainParticipantImpl participant,
        String _name,
        TopicImpl _related_topic,
        String _expression,
        String[] _parameters)
    {
        int result = DDS.RETCODE_BAD_PARAMETER.value;

        if (_name != null && _related_topic != null && participant != null) {
            result = _related_topic.validate_filter(_expression, _parameters);
        }
        if (result == DDS.RETCODE_OK.value) {
            this.name = _name;
            this.relatedTopic = _related_topic;
            this.expression = _expression;
            this.parameters = _parameters;
            this.participant = participant;
            this.relatedTopic.keep(); /* Topic cannot be deleted, first undo keep by free. */
            this.setDomainId(_related_topic.getDomainId());
        }
        return result;
    }

    @Override
    protected int deinit ()
    {
        int result = DDS.RETCODE_OK.value;

        synchronized (this)
        {
            if (this.refCount == 0) {
                this.relatedTopic.free(); /* release topic */

                this.name = null;
                this.expression = null;
                this.parameters = null;
                this.relatedTopic = null;
                this.participant = null;
                this.deleted = true;
            } else {
                result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
            }
        }
        return result;
    }

    @Override
    public int keep()
    {
        int result = DDS.RETCODE_OK.value;

        synchronized (this)
        {
            if (!deleted) {
                this.refCount++;
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }
        return result;
    }

    @Override
    public int free()
    {
        int result = DDS.RETCODE_OK.value;

        synchronized (this)
        {
            if (!deleted) {
                this.refCount--;
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }
        return result;
    }

    @Override
    public String get_type_name () {
        String type_name = null;
        TopicImpl relatedTopic = this.relatedTopic;

        /* no synchronization required; if relatedTopic = null then already deleted and thus return null. */
        if (relatedTopic != null) {
            type_name = relatedTopic.get_type_name();
        }
        return type_name;
    }

    @Override
    public String get_name () {
        return name;
    }

    @Override
    public DDS.DomainParticipant get_participant () {
        return participant;
    }

    @Override
    public String get_filter_expression () {
        return expression;
    }

    /* see DDS.ContentFilteredTopicOperations for javadoc */
    @Override
    public int get_expression_parameters (DDS.StringSeqHolder expression_parameters) {
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        synchronized (this)
        {
            if (!deleted) {
                if (expression_parameters != null) {
                    expression_parameters.value = this.parameters;
                    result = DDS.RETCODE_OK.value;
                } else {
                    result = DDS.RETCODE_BAD_PARAMETER.value;
                }
            }
        }
        return result;
    }

    /* see DDS.ContentFilteredTopicOperations for javadoc */
    @Override
    public int set_expression_parameters (String[] expression_parameters) {
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        synchronized (this)
        {
            if (!deleted) {
                if (expression_parameters != null) {
                    this.parameters = expression_parameters;
                    result = DDS.RETCODE_OK.value;
                } else {
                    result = DDS.RETCODE_BAD_PARAMETER.value;
                }
            }
        }
        return result;
    }

    /* see DDS.ContentFilteredTopicOperations for javadoc */
    @Override
    public DDS.Topic get_related_topic () {
        return relatedTopic;
    }

    @Override
    public DDS.DataReader create_datareader ()
    {
        DDS.DataReader reader = null;
        TopicImpl relatedTopic = this.relatedTopic;

        /* no synchronization required; if relatedTopic = null then already deleted and thus return null. */
        if (relatedTopic != null) {
            reader = relatedTopic.create_datareader();
        }
        return reader;
    }

    @Override
    public DDS.DataReaderView create_dataview ()
    {
        DDS.DataReaderView view = null;
        TopicImpl relatedTopic = this.relatedTopic;

        /* no synchronization required; if relatedTopic = null then already deleted and thus return null. */
        if (relatedTopic != null) {
            view = relatedTopic.create_dataview();
        }
        return view;
    }

}
