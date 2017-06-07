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
 */
package org.vortex.FACE;

import java.util.HashMap;
import java.util.Map;

import FACE.RETURN_CODE_TYPE;
import FACE.RETURN_CODE_TYPEHolder;

public class TSFactory {
    private Map<String, TransportServices> tsses;
    private TSFactory() {
        this.tsses = new HashMap<String, TransportServices>();
    }

    private static class LazyHolder {
        private static final TSFactory INSTANCE = new TSFactory();
    }

    public static TSFactory getInstance() {
        return LazyHolder.INSTANCE;
    }

    public void getTS(String configuration, Holder<TransportServices> tsHolder,
            RETURN_CODE_TYPEHolder holder) {
        synchronized(this.tsses){
            TransportServices ts = this.tsses.get(configuration);

            if (ts == null) {
                ts = new TransportServices();
                ts.Initialize(configuration, holder);

                if (holder.value == RETURN_CODE_TYPE.NO_ERROR) {
                    this.tsses.put(configuration, ts);
                    tsHolder.value = ts;
                }
            } else {
                tsHolder.value = ts;
                holder.value = RETURN_CODE_TYPE.NO_ACTION;
            }
        }
    }
}
