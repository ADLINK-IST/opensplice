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

import org.opensplice.dds.dcps.ReportStack;

/**
 * This ultimate Java language binding base class is implemented by all DDS defined
 * classes and holds the adress of the equivalent <code>user-layer</code> object.
 * The adress is stored as a <code>long</code> because the <code>user-layer</code> can be
 * compiled for any platform using up to 64 bit adressing.
 */
abstract public class ObjectImpl extends ObjectBase {

   /**
    * The adress of the equivalent object in the <code>user_layer</code>
    */
    private long uObject = 0;
    private int invalid = 0;
    private int domainId = DDS.DOMAIN_ID_INVALID.value;

    @Override
    protected void finalize() throws Throwable {
        if (this.uObject != 0) {
            jniUObjectFree(this.uObject);
        }
    }

    protected int deinit() {
        this.invalid = 1;
        return DDS.RETCODE_OK.value;
    }

    protected void set_user_object(long uObject) {
        if (this.uObject == 0) {
            this.uObject = uObject;
        } else {
            ReportStack.report (DDS.RETCODE_ERROR.value, "User object already set.");
        }
    }

    protected long get_user_object() {
        long object = 0;
        if (this.invalid == 0) {
            object = this.uObject;
        } else {
            ReportStack.report(DDS.RETCODE_ALREADY_DELETED.value,
                    "Entity already deleted.");
        }
        return object;
    }

    protected int getDomainId() {
        return this.domainId;
    }

    protected void setDomainId(int domainId) {
        this.domainId = domainId;
    }

    private native int jniUObjectFree(long uObject);
}
