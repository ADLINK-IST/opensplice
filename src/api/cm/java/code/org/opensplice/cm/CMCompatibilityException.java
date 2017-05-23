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

package org.opensplice.cm;

/**
 * Control & Monitoring Compatibility exception.
 *
 * The exception provides information about the CM API compatibility exception that occurred. This
 * message can be accessed by calling the getMessage function.
 */
public class CMCompatibilityException extends CMException {

    private int localMajor = VERSION_NOT_FOUND;
    private int localMinor = VERSION_NOT_FOUND;
    private int remoteMajor = VERSION_NOT_FOUND;
    private int remoteMinor = VERSION_NOT_FOUND;

    public static final int VERSION_NOT_FOUND = -1;

    private static final long serialVersionUID = 1867756917914320490L;

    public CMCompatibilityException(String message, int localMajor, int localMinor, int remoteMajor, int remoteMinor) {
        super(String.format(message, localMajor, localMinor, remoteMajor, remoteMinor));

        this.localMajor = localMajor;
        this.localMinor = localMinor;
        this.remoteMajor = remoteMajor;
        this.remoteMinor = remoteMinor;
    }

    public CMCompatibilityException(String message) {
        super(message);
    }

    public int getLocalMajor() {
        return localMajor;
    }

    public int getLocalMinor() {
        return localMinor;
    }

    public int getRemoteMajor() {
        return remoteMajor;
    }

    public int getRemoteMinor() {
        return remoteMinor;
    }
}
