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
 * Implementation of the {@link DDS.Domain} interface. 
 */ 
public class DomainImpl extends DomainBase implements DDS.Domain {
    private static final long serialVersionUID = 6683917520488717432L;
    private int domainId = 0;

    protected DomainImpl () { }

    protected int init (int domainId)
    {
        int result;
        long uDomain = jniDomainNew(domainId);
        if (uDomain != 0) {
            this.set_user_object(uDomain);
            this.domainId = domainId;
            this.setDomainId(domainId);
            result = DDS.RETCODE_OK.value;
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                    "Could not locate a domain with domainId = " + domainId + ".");
        }
        return result;
    }

    @Override
    protected int deinit ()
    {
        long uDomain = 0;
        int result = DDS.RETCODE_ALREADY_DELETED.value;

        uDomain = this.get_user_object();
        if (uDomain != 0) {
            result = jniDomainFree(uDomain);
            if (result == DDS.RETCODE_OK.value) {
                result = super.deinit();
            }
        }
        return result;
    }

    public int get_domain_id()
    {
        return this.domainId;
    }

    /* see DDS.DomainOperations for javadoc */ 
    @Override
    public int create_persistent_snapshot (
        String partition_expression,
        String topic_expression,
        String URI)
    {
        int result = DDS.RETCODE_OK.value;
        long uDomain = 0;
        ReportStack.start();

        if (partition_expression == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "partition_expression 'null' is invalid.");
        } else if (topic_expression == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "topic_expression 'null' is invalid.");
        } else if (URI == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "URI 'null' is invalid.");
        } else {
            uDomain = this.get_user_object();
            if (uDomain != 0) {
                result = jniCreatePersistentSnapshot(
                    uDomain, partition_expression, topic_expression, URI);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    private native long jniDomainNew(int domainId);
    private native int jniDomainFree(long uDomain);
    private native int jniDomainId(long uDomain);

    private native int jniCreatePersistentSnapshot(long uDomain, String partition_expression,
                                                   String topic_expression, String URI);
}
