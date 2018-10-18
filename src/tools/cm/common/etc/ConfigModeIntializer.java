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
package org.opensplice.common.util;

import java.io.File;

public class ConfigModeIntializer {

    public static final String COMMUNITY = "COMMUNITY";
    public static final String COMMERCIAL = "COMMERCIAL";
    public static final int  COMMUNITY_MODE = 1;
    public static final int  COMMERCIAL_MODE = 2;
    public static final int  COMMUNITY_MODE_FILE_OPEN = 3;
    public static final int LITE_MODE = 4;
    public static int          CONFIGURATOR_MODE        = COMMUNITY_MODE;

    public static void setMode(int mode) {
        CONFIGURATOR_MODE = mode;
    }

    public int getMode() {
        return CONFIGURATOR_MODE;
    }
}
